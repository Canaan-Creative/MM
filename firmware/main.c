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

static void gen_hash(uint8_t *data, uint8_t *hash, size_t len)
{
}

static void calc_midstate(struct work *work)
{
}

static void gen_work(struct mm_work *mw, struct work *work)
{
	uint8_t merkle_root[32];

	memcpy(mw->coinbase + mw->nonce2_offset, (uint8_t *)(&mw->nonce2), sizeof(uint32_t));
	work->nonce2 = mw->nonce2++;

	gen_hash(mw->coinbase, merkle_root, mw->coinbase_len);

	calc_midstate(work);
}

int main(void) {
	uart_init();
	serial_puts(MM_VERSION);

	/* Test sha256 core: 1 block data*/
	uint8_t state[32];
	const uint8_t sha256_in[3] = {0x61, 0x62, 0x63};
	const uint8_t sha256_in2[128] = {
		0x61, 0x62, 0x63, 0x64, 0x62, 0x63, 0x64, 0x65, 0x63, 0x64, 0x65, 0x66, 0x64, 0x65, 0x66, 0x67,
		0x65, 0x66, 0x67, 0x68, 0x66, 0x67, 0x68, 0x69, 0x67, 0x68, 0x69, 0x6A, 0x68, 0x69, 0x6A, 0x6B,
		0x69, 0x6A, 0x6B, 0x6C, 0x6A, 0x6B, 0x6C, 0x6D, 0x6B, 0x6C, 0x6D, 0x6E, 0x6C, 0x6D, 0x6E, 0x6F,
		0x6D, 0x6E, 0x6F, 0x70, 0x6E, 0x6F, 0x70, 0x71};
	sha256(state, sha256_in, ARRAY_SIZE(sha256_in));
	hexdump(state, 32);

	/* Test sha256 core: 2 block data*/
	sha256(state, sha256_in2, ARRAY_SIZE(sha256_in));
	hexdump(state, 32);

#include "cb_test.c"
	mm_work.coinbase = cb;
	mm_work.coinbase_len = ARRAY_SIZE(cb);
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
	mm_work.nonce2_offset = 119;
	mm_work.nonce2_size = 4;
	mm_work.nonce2 = 0;

	gen_work(&mm_work, &work);

	/* Code should be never reach here */
	error(0xf);
	return 0;
}

/* vim: set ts=4 sw=4 fdm=marker : */
