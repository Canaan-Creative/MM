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

struct mm_work mm_work;
struct work work[8];

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

static void calc_midstate(struct work *work)
{
	unsigned char data[64];
	uint32_t *data32 = (uint32_t *)data;

	flip64(data32, work->data);

	sha256_init();
	sha256_update(data, 64);
	sha256_final(work->midstate);

	flip64(work->midstate, work->midstate);
}

static void gen_work(struct mm_work *mw, struct work *work)
{
	uint8_t merkle_root[32], merkle_sha[64];
	uint32_t *data32, *swap32, tmp32;
	int i;

	tmp32 = mw->nonce2;
	tmp32 = bswap_32(tmp32);
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

	memcpy(work->data, mw->header, 128);
	memcpy(work->data + mw->merkle_offset, merkle_root, 32);

	debug32("Generated merkle_root:\n"); hexdump(merkle_root, 32);
	debug32("Generated header:\n"); hexdump(work->data, 128);
	debug32("Work job_id nonce2 ntime \n"); hexdump((uint8_t *)(&work->nonce2), 4);
	calc_midstate(work);
}

/* Total: 4W + 19W = 23W
 * TaskID_H:1, TASKID_L:1, STEP:1, TIMEOUT:1,
 * CLK_CFG:2, a2, Midsate:8, e0, e1, e2, a0, a1, Data:3
 */
void send_work(struct work *w)
{
}

int main(void) {
	int i;

	uart_init();
	serial_puts(MM_VERSION);

#include "sha256_test.c"
#include "cb_test1.c"
	for (i = 0; i < 8; i++)
		gen_work(&mm_work, &work[i]);

	send_work(&work[0]);
	/* Code should be never reach here */
	error(0xf);
	return 0;
}

/* vim: set ts=4 sw=4 fdm=marker : */
