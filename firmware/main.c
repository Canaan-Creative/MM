/*
 * Author: Minux
 * Author: Xiangfu Liu <xiangfu@openmobilefree.net>
 * Bitcoin:	1CanaaniJzgps8EV6Sfmpb7T8RutpaeyFn
 *
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */

#include <stdbool.h>

#include "sdk.h"
#include "minilibc.h"

#include "system_config.h"
#include "defines.h"

#include "io.h"
#include "serial.h"
#include "miner.h"

#include "hexdump.c"

#ifdef DEBUG
char printf_buf32[32];
#define debug32(...)	do {				\
		m_sprintf(printf_buf32, __VA_ARGS__);	\
		serial_puts(printf_buf32);		\
	} while(0)
#else
#define debug32(...)
#endif

struct mm_work mm_work;
struct work work;

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

const uint32_t sha256_in[16] = {
	0x61626380, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000018};

const uint32_t sha256_in2[32] = {
	0x61626364, 0x62636465, 0x63646566, 0x64656667,
	0x65666768, 0x66676869, 0x6768696A, 0x68696A6B,
	0x696A6B6C, 0x6A6B6C6D, 0x6B6C6D6E, 0x6C6D6E6F,
	0x6D6E6F70, 0x6E6F7071, 0x80000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x000001C0};

static void sha256_transform(uint32_t *state, const uint32_t *input, int count)
{
	struct lm32_sha256 *sha256 = (struct lm32_sha256 *)SHA256_BASE;

	int i;

	writel(LM32_SHA256_CMD_INIT, &sha256->cmd);
	for (i = 0; i < count; i++) {
		writel(input[i], &sha256->in);
		if (!((i + 1) % 16))
			while (!(readl(&sha256->cmd) & LM32_SHA256_CMD_DONE))
				;
	}
	for (i = 0; i < 8; i++)
		state[i] = readl(&sha256->out);
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

#include "cb_test.c"

static void calc_midstate(struct work *work)
{
}

static void gen_stratum_work(struct mm_work *mw, struct work *work)
{

	calc_midstate(work);
}

int main(void) {
	uint32_t state[8];

	uart_init();
	serial_puts(MM_VERSION);

	/* Test sha256 core: 1 block data*/
	sha256_transform(state, sha256_in, 16);
	hexdump((uint8_t *)state, 32);

	/* Test sha256 core: 2 block data*/
	sha256_transform(state, sha256_in2, 32);
	hexdump((uint8_t *)state, 32);

	mm_work.coinbase = cb;
	mm_work.merkels[0] = m0;
	mm_work.merkels[1] = m1;
	mm_work.merkels[2] = m2;
	mm_work.merkels[3] = m3;
	mm_work.merkels[4] = m4;
	mm_work.merkels[5] = m5;
	mm_work.merkels[6] = m6;
	mm_work.merkels[7] = m7;
	mm_work.merkels[8] = m8;
	mm_work.merkels[9] = m9;
	mm_work.header = h;

	mm_work.cb_nonce2_offset = 0;
	mm_work.cb_nonce2_size = 4;

	gen_stratum_work(&mm_work, &work);

	/* Code should be never reach here */
	error(0xf);
	return 0;
}

/* vim: set ts=4 sw=4 fdm=marker : */
