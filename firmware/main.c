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

#include "hexdump.c"

#define WORK_BUF_LEN	(8)
#define adjust_fan(value)	write_pwm(value)

struct mm_work mm_work;
struct work work[WORK_BUF_LEN];
struct result result;


uint8_t pkg[P_COUNT];
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

static void decode_pkg(uint8_t *p, struct mm_work *mw)
{
	debug32("p[0]: %d\n", p[0]);
	switch (p[0]) {
	case 86: {
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
	}

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
			if (heada == H1 && headv == H2) {
				uart_write('U');
				start = 1;
				count = 0;
			}

			if (start)
				pkg[count++] = c;

			tailo = tailn;
			tailn = c;
			if (tailo == T1 && tailn == T2) {
				uart_write('I');
				start = 0;
				if (count - 1 == P_COUNT) {
					decode_pkg(pkg, &mm_work);
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
