/*
 * Author: Minux
 * Author: Xiangfu Liu <xiangfu@openmobilefree.net>
 * Bitcoin:	1CanaaniJzgps8EV6Sfmpb7T8RutpaeyFn
 *
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */

#include <stdbool.h>
#include <stdint.h>

#include "sdk.h"
#include "minilibc.h"

#include "system_config.h"
#include "defines.h"

#include "io.h"
#include "serial.h"
#include "miner.h"
#include "sha256.h"

#include "hexdump.c"

#define WORK_BUF_LEN	(10)

struct mm_work mm_work;
struct work work[WORK_BUF_LEN];
struct result result;

uint8_t pkg[40];
uint8_t buffer[4*1024];

static void delay(volatile uint32_t i)
{
	while (i--)
		;
}

static void error(uint8_t n)
{
	volatile uint32_t *gpio = (uint32_t *)GPIO_BASE;
	uint8_t i = 0;

	while (1) {
		delay(4000000);
		if (i++ %2)
			writel(0x00000000 | (n << 24), gpio);
		else
			writel(0x00000000, gpio);
	}
}

static int bin_value(unsigned char ch)
{
        if ('0' <= ch && ch <= '9')
                return ch - '0';
        else if ('a' <= ch && ch <= 'f')
                return ch - 'a' + 0x0A;
        else if ('A' <= ch && ch <= 'F')
                return ch - 'A' + 0x0A;
        else
                return -1;
}

bool hex2bin(unsigned char *p, const char *hexstr, size_t len)
{
	int a, b;

	while (*hexstr && len) {
		a = bin_value(hexstr[0]);
		b = bin_value(hexstr[1]);
		if (a == -1 || b == -1) {
			serial_puts("E: hex2bin failed:");
			serial_putc(hexstr[0]);
			serial_putc(hexstr[1]);
			serial_putc('\n');
			return false;
		}

		a = ((a<<4) & 0xF0);
		b = ((b   ) & 0x0F);
		*p = (unsigned char)(a | b);

		hexstr += 2;
		p++;
		len--;
	}

	if (len == 0)
		return true;
	return false;
}

static void flip32(void *dest_p, const void *src_p)
{
	uint32_t *dest = dest_p;
	const uint32_t *src = src_p;
	int i;

	for (i = 0; i < 8; i++)
		dest[i] = bswap_32(src[i]);
}

static void flip64(void *dest_p, const void *src_p)
{
	uint32_t *dest = dest_p;
	const uint32_t *src = src_p;
	int i;

	for (i = 0; i < 16; i++)
		dest[i] = bswap_32(src[i]);
}

static void gen_hash(uint8_t *data, uint8_t *hash, unsigned int len)
{
	uint8_t hash1[32];

	sha256(data, len, hash1);
	sha256(hash1, 32, hash);
}

static void init_work(struct mm_work *mw, struct work *work)
{
	/* TODO: create the task_id */
	work->timeout[0] = 0xff;
	work->timeout[1] = 0xff;
	work->timeout[2] = 0xff;
	work->timeout[3] = 0xff;

	work->clock[0] = 0x94;
	work->clock[1] = 0xe0;
	work->clock[2] = 0x00;
	work->clock[3] = 0x01;
	work->clock[4] = 0x00;
	work->clock[5] = 0x00;
	work->clock[6] = 0x00;
	work->clock[7] = 0x00;

	work->step[0] = 0x19;
	work->step[1] = 0x99;
	work->step[2] = 0x99;
	work->step[3] = 0x99;
}

static void calc_midstate(struct mm_work *mw, struct work *work)
{
	unsigned char data[64];
	uint32_t *data32 = (uint32_t *)data;

	flip64(data32, mw->header);

	sha256_init();
	sha256_update(data, 64);
	sha256_final(work->data);

	flip64(work->data, work->data);
	memcpy(work->data + 32, mw->header + 64, 12);
}

/* Total: 4W + 19W = 23W
 * TaskID_H:1, TASKID_L:1, STEP:1, TIMEOUT:1,
 * CLK_CFG:2, a2, Midsate:8, e0, e1, e2, a0, a1, Data:3
 */
static void calc_prepare(struct mm_work *mw, struct work *work)
{
	uint32_t precalc[6];
	sha256_precalc(work->data, 44, (uint8_t *)precalc);
	memcpy(work->a0, precalc + 0, 4);
	memcpy(work->a1, precalc + 1, 4);
	memcpy(work->a2, precalc + 2, 4);
	memcpy(work->e0, precalc + 3, 4);
	memcpy(work->e1, precalc + 4, 4);
	memcpy(work->e2, precalc + 5, 4);
}

static void gen_work(struct mm_work *mw, struct work *work)
{
	uint8_t merkle_root[32], merkle_sha[64];
	uint32_t *data32, *swap32, tmp32;
	int i;

	tmp32 = bswap_32(mw->nonce2);
	memcpy(mw->coinbase + mw->nonce2_offset, (uint8_t *)(&tmp32), sizeof(uint32_t));
	work->nonce2 = mw->nonce2++;

	gen_hash(mw->coinbase, merkle_root, mw->coinbase_len);
	memcpy(merkle_sha, merkle_root, 32);
	for (i = 0; i < mw->nmerkles; i++) {
		memcpy(merkle_sha + 32, mw->merkles[i], 32);
		gen_hash(merkle_sha, merkle_root, 64);
		memcpy(merkle_sha, merkle_root, 32);
	}
	data32 = (uint32_t *)merkle_sha;
	swap32 = (uint32_t *)merkle_root;
	flip32(swap32, data32);

	memcpy(mw->header + mw->merkle_offset, merkle_root, 32);

	debug32("Work nonce2:\n"); hexdump((uint8_t *)(&work->nonce2), 4);
	debug32("Generated header:\n"); hexdump(mw->header, 128);
	calc_midstate(mw, work);
	calc_prepare(mw, work);
}

struct lm32_alink *alink = (struct lm32_alink *)ALINK_BASE;
int alink_full()
{
	return (LM32_ALINK_STATE_TXFULL & alink->state);
}

void send_work(struct work *w)
{
	uint32_t tmp;
	int i;

	while (alink_full()) {};

	memcpy((uint8_t *)(&tmp), w->task_id, 4);
	writel(tmp, alink->tx);

	memcpy((uint8_t *)(&tmp), w->task_id + 4, 4);
	writel(tmp, alink->tx);

	memcpy((uint8_t *)(&tmp), w->step, 4);
	writel(tmp, alink->tx);

	memcpy((uint8_t *)(&tmp), w->timeout, 4);
	writel(tmp, alink->tx);

	memcpy((uint8_t *)(&tmp), w->clock, 4);
	writel(tmp, alink->tx);

	memcpy((uint8_t *)(&tmp), w->clock + 4, 4);
	writel(tmp, alink->tx);

	memcpy((uint8_t *)(&tmp), w->a2, 4);
	writel(tmp, alink->tx);

	for (i = 0; i < 32 / 4; i += 4) {
		memcpy((uint8_t *)(&tmp), w->data + i, 4);
		writel(tmp, alink->tx);
	}

	memcpy((uint8_t *)(&tmp), w->e0, 4);
	writel(tmp, alink->tx);
	memcpy((uint8_t *)(&tmp), w->e1, 4);
	writel(tmp, alink->tx);
	memcpy((uint8_t *)(&tmp), w->e2, 4);
	writel(tmp, alink->tx);
	memcpy((uint8_t *)(&tmp), w->a0, 4);
	writel(tmp, alink->tx);
	memcpy((uint8_t *)(&tmp), w->a1, 4);
	writel(tmp, alink->tx);

	for (i = 0; i < 12 / 4; i += 4) {
		memcpy((uint8_t *)(&tmp), w->data + 32 + i, 4);
		writel(tmp, alink->tx);
	}
}

static void submit_result()
{
	hexdump((uint8_t *)(&result), 20);
}

static void read_result()
{
	uint32_t tmp;

	while (!(LM32_ALINK_STATE_RXEMPTY & alink->state)) {
		tmp = readl(alink->rx);
		memcpy(result.miner_id, (uint8_t *)(&tmp), 4);

		tmp = readl(alink->rx);
		memcpy(result.task_id, (uint8_t *)(&tmp), 4);

		tmp = readl(alink->rx);
		memcpy(result.timeout, (uint8_t *)(&tmp), 4);

		tmp = readl(alink->rx);
		memcpy(result.nonce, (uint8_t *)(&tmp), 4);

		/* TODO: test the result before submit */
		submit_result();
	}
}

static void decode_package(uint8_t *buf)
{
	int i = 0, j = ARRAY_SIZE(pkg);

	while (j--) {
		buffer[i] = pkg[i];
		i++;
	}
}

static void get_package()
{
	int i = 0, j = ARRAY_SIZE(pkg);

	while (j--) {
		pkg[i++] = serial_getc();
	}
}

int main(void) {
	int i;

	uart_init();
	serial_puts(MM_VERSION);

	while (1) {
		get_package();
		decode_package(pkg);

#include "sha256_test.c"
#include "cb_test1.c"

		for (i = 0; i < WORK_BUF_LEN; i++) {
			read_result();
			/* TODO: try to read result here */
			init_work(&mm_work, &work[i]);
			gen_work(&mm_work, &work[i]);
			send_work(&work[i]);
		}
	}

	/* Code should be never reach here */
	error(0xf);
	return 0;
}

/* vim: set ts=4 sw=4 fdm=marker : */
