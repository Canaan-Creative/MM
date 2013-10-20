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
#include "serial.h"
#include "miner.h"
#include "sha256.h"
#include "alink.h"
#include "twipwm.h"

#include "hexdump.c"

#define WORK_BUF_LEN	(8)
#define adjust_fan(value)	write_pwm(value)

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


static void get_package()
{
	int i = 0, j = ARRAY_SIZE(pkg);

	while (j--) {
		pkg[i] = serial_getc();
		serial_putc(pkg[i]);
		i++;
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
		serial_getc();
		submit_result(&result);
	}
}


int main(int argv, char **argc) {
	int i;

	irq_setmask(0);
	irq_enable(1);

	uart_init();
	debug32("%s\n", MM_VERSION);

	alink_init(0xff);
	adjust_fan(0xff);

#include "sha256_test.c"
#include "cb_test1.c"
	/* TEST:  */
	get_package();

	/* TEST: PHY functions */
	for (i = 0; i < WORK_BUF_LEN; i++)
		send_test_work(i);
	while (1)
		read_result();

	/* TEST: PHY with more data */
	while (1) {
		for (i = 0; i < WORK_BUF_LEN; i++) {
			/* TODO: try to read result here */
			miner_init_work(&mm_work, &work[i]);
			miner_gen_work(&mm_work, &work[i]);
			alink_send_work(&work[i]);
			read_result();
		}
		serial_getc();
	}
	error(0xf);
	return 0;
}

/* vim: set ts=4 sw=4 fdm=marker : */
