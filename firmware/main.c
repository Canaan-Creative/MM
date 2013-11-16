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

#include "minilibc.h"
#include "system_config.h"
#include "defines.h"
#include "io.h"
#include "intr.h"
#include "uart.h"
#include "miner.h"
#include "sha256.h"
#include "alink.h"
#include "twipwm.h"
#include "protocol.h"
#include "crc.h"

#include "hexdump.c"

#define WORK_BUF_LEN	(8)
#define adjust_fan(value)	write_pwm(value)

struct mm_work mm_work;
struct work work[WORK_BUF_LEN];
struct result result;

uint8_t g_pkg[AVA2_P_COUNT - 2];
uint8_t g_act[AVA2_P_COUNT];
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

	while (i < 8) {
		delay(4000000);
		if (i++ % 2)
			writel(n << 24, gpio);
		else
			writel(0, gpio);
	}
}

static void encode_pkg(uint8_t *p, int type)
{
	int i;
	uint16_t crc;

	p[0] = AVA2_H1;
	p[1] = AVA2_H2;
	p[AVA2_P_COUNT - 2] = AVA2_T1;
	p[AVA2_P_COUNT - 1] = AVA2_T2;

	p[2] = type;
	p[3] = 1;
	p[4] = 1;

	switch(type) {
	case AVA2_P_ACKDETECT:
		for (i = 0; i < 32; i++)
			p[i + 5] = i;
		break;
	}

	crc = crc16(p + 5, 32);
	p[AVA2_P_COUNT - 4] = crc & 0x00ff;
	p[AVA2_P_COUNT - 3] = (crc & 0xff00) >> 8;

	hexdump(p, AVA2_P_COUNT);
}

static void decode_pkg(uint8_t *p, struct mm_work *mw)
{
	hexdump(p, AVA2_P_COUNT - 2);
	switch (p[0]) {
	case 0:
		encode_pkg(g_act, AVA2_P_ACKDETECT);
		uart_nwrite((char *)g_act, AVA2_P_COUNT);
		break;
	case 1:
		memcpy((uint8_t *)mw->coinbase_len, p + 1, 4);
		memcpy((uint8_t *)mw->nonce2_offset, p + 5, 4);
		memcpy((uint8_t *)mw->nonce2_size, p + 9, 4);
		memcpy((uint8_t *)mw->merkle_offset, p + 13, 4);
		memcpy((uint8_t *)mw->nmerkles, p + 17, 4);
		debug32("dpkg: %d, %d, %d, %d, %d\n",
			mw->coinbase_len,
			mw->nonce2_offset,
			mw->nonce2_size,
			mw->merkle_offset,
			mw->nmerkles);
		break;
	default:
		break;
	}
}

static void get_pkg()
{
	static char heada, headv;
	static char tailo, tailn;
	static int start, count;
	char c;

	while (1) {
		if (uart_read_nonblock()) {
			c = uart_read();

			heada = headv;
			headv = c;
			if (heada == AVA2_H1 && headv == AVA2_H2) {
				start = 1;
				count = 0;
				continue;
			}

			if (start)
				g_pkg[count++] = c;

			tailo = tailn;
			tailn = c;
			if (tailo == AVA2_T1 && tailn == AVA2_T2) {
				start = 0;
				if (count == AVA2_P_COUNT - 2) {
					decode_pkg(g_pkg, &mm_work);
					/* Send back ACK */
				} else {
					debug32("E: package broken: %d\n", count);
					/* Send back RESEND */
				}
			}
		} else
			break;
	}

}

static void submit_result(struct result *r)
{
	hexdump((uint8_t *)(r), 20);
}

static void read_result()
{
	while(!alink_rxbuf_empty()) {
		debug32("Found nonce\n");
		alink_buf_status();
		alink_read_result(&result);
		submit_result(&result);
	}
}


int main(int argv, char **argc) {
	int i;

	irq_setmask(0);
	irq_enable(1);

	uart_init();
	uart_force_sync(1);

	debug32("%s\n", MM_VERSION);

	alink_init(0xff);
	adjust_fan(0xff);

	while (1) {
		get_pkg();
	}

#include "sha256_test.c"
#include "cb_test1.c"
#include "alink_test.c"

	while (1) {
		get_pkg();
		for (i = 0; i < WORK_BUF_LEN; i++) {
			miner_init_work(&mm_work, &work[i]);
			miner_gen_work(&mm_work, &work[i]);
			alink_send_work(&work[i]);
			read_result();
		}
	}

	error(0xf);
	return 0;
}

/* vim: set ts=4 sw=4 fdm=marker : */
