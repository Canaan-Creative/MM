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
#include <stddef.h>

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
#include "shifter.h"
#include "protocol.h"
#include "crc.h"

#include "hexdump.c"

static uint8_t g_pkg[AVA2_P_COUNT];
static uint8_t g_act[AVA2_P_COUNT];
static int g_new_stratum = 0;

void delay(unsigned int ms)
{
	unsigned int i;

	while (ms--) {
		for (i = 0; i < CPU_FREQUENCY / 1000 / 5; i++)
			__asm__ __volatile__("nop");
	}
}

static void led(uint8_t value)
{
	volatile uint32_t *gpio = (uint32_t *)GPIO_BASE;

	writel(value << 24, gpio);
}

static void encode_pkg(uint8_t *p, int type, uint8_t *buf, unsigned int len)
{
	uint32_t tmp;
	uint16_t crc;

	memset(p, 0, AVA2_P_COUNT);

	p[0] = AVA2_H1;
	p[1] = AVA2_H2;
	p[AVA2_P_COUNT - 2] = AVA2_T1;
	p[AVA2_P_COUNT - 1] = AVA2_T2;

	p[2] = type;
	p[3] = 1;
	p[4] = 1;

	switch(type) {
	case AVA2_P_ACKDETECT:
		p[5 + 0] = 'M';
		p[5 + 1] = 'M';
		memcpy(p + 5 + 2, MM_VERSION, 6);
		break;
	case AVA2_P_NONCE:
		memcpy(p + 5, buf, len);
		break;
	case AVA2_P_REQUIRE:
		tmp = read_temp0();
		memcpy(p + 5 + 0, &tmp, 4);
		tmp = read_temp1();
		memcpy(p + 5 + 4, &tmp, 4);
		tmp = read_fan0();
		memcpy(p + 5 + 8, &tmp, 4);
		tmp = read_fan1();
		memcpy(p + 5 + 12, &tmp, 4);
		break;
	}

	crc = crc16(p + 5, AVA2_P_DATA_LEN);
	p[AVA2_P_COUNT - 4] = crc & 0x00ff;
	p[AVA2_P_COUNT - 3] = (crc & 0xff00) >> 8;
}

static void send_pkg(int type, uint8_t *buf, unsigned int len)
{
	encode_pkg(g_act, type, buf, len);
	uart_nwrite((char *)g_act, AVA2_P_COUNT);
}

static int decode_pkg(uint8_t *p, struct mm_work *mw)
{
	unsigned int expected_crc;
	unsigned int actual_crc;
	int idx, cnt;
	uint32_t tmp;

	uint8_t *data = p + 5;

	idx = p[3];
	cnt = p[4];

	debug32("Decode: %d: %d/%d\n", p[2], idx, cnt);

	expected_crc = (p[AVA2_P_COUNT - 3] & 0xff) |
		((p[AVA2_P_COUNT - 4] & 0xff) << 8);

	actual_crc = crc16(data, AVA2_P_DATA_LEN);
	if(expected_crc != actual_crc) {
		debug32("PKG CRC failed (expected %08x, got %08x)\n",
			expected_crc, actual_crc);
		return 1;
	}

	switch (p[2]) {
	case AVA2_P_DETECT:
		alink_flush_fifo(); /* Ignore all result and task when new stratum come */
		g_new_stratum = 0;
		break;
	case AVA2_P_STATIC:
		memcpy(&mw->coinbase_len, data, 4);
		memcpy(&mw->nonce2_offset, data + 4, 4);
		memcpy(&mw->nonce2_size, data + 8, 4);
		memcpy(&mw->merkle_offset, data + 12, 4);
		memcpy(&mw->nmerkles, data + 16, 4);
		memcpy(&mw->diff, data + 20, 4);
		g_new_stratum = 0;
		debug32("D: (%d):  %d, %d, %d, %d, %d, %d\n",
			g_new_stratum,
			mw->coinbase_len,
			mw->nonce2_offset,
			mw->nonce2_size,
			mw->merkle_offset,
			mw->nmerkles,
			mw->diff);
		break;
	case AVA2_P_JOB_ID:
		break;
	case AVA2_P_COINBASE:
		if (idx == 1)
			memset(mw->coinbase, 0, sizeof(mw->coinbase));
		memcpy(mw->coinbase + (idx - 1) * AVA2_P_DATA_LEN, data, AVA2_P_DATA_LEN);
		g_new_stratum = 0;
		break;
	case AVA2_P_MERKLES:
		memcpy(mw->merkles[idx - 1], data, AVA2_P_DATA_LEN);
		g_new_stratum = 0;
		break;
	case AVA2_P_HEADER:
		memcpy(mw->header + (idx - 1) * AVA2_P_DATA_LEN, data, AVA2_P_DATA_LEN);
		if (idx == cnt) {
			g_new_stratum = 1;
			debug32("D: Header(%d)\n", g_new_stratum);
		}
		break;
	case AVA2_P_POLLING:
	case AVA2_P_DIFF:
	case AVA2_P_SET:
		memcpy(&tmp, data, 4);
		adjust_fan(tmp);
		memcpy(&tmp, data + 4, 4);
		adjust_voltage(tmp);
		memcpy(&tmp, data + 8, 4);
		adjust_freq(tmp);
	default:
		break;
	}

	return 0;
}

static int test_nonce(struct result *ret)
{

	return 0;
}

static int read_result(struct result *ret)
{
	if (alink_rxbuf_empty())
		return 0;

#ifdef DEBUG
	alink_buf_status();
#endif

	alink_read_result(ret);
	if (!test_nonce(ret)) {
		    send_pkg(AVA2_P_NONCE, (uint8_t *)ret, 20);
		    return 2;
	}

	return 1;
}

static int get_pkg(struct mm_work *mw)
{
	static char pre_last, last;
	static int start = 0, count = 2;

	while (1) {
		if (!uart_read_nonblock() && !start)
			break;

		pre_last = last;
		last = uart_read();

		if (start)
			g_pkg[count++] = last;

		if (count == AVA2_P_COUNT) {
			start = 0;
			count = 2;
			if (pre_last == AVA2_T1 && last == AVA2_T2) {
				if (decode_pkg(g_pkg, mw)) {
					debug32("E: package broken(crc)\n");
					send_pkg(AVA2_P_NAK, NULL, 0);
					return 1;
				} else {
					send_pkg(AVA2_P_ACK, NULL, 0);
					switch (g_pkg[2]) {
					case AVA2_P_DETECT:
						send_pkg(AVA2_P_ACKDETECT, (uint8_t *)MM_VERSION, 6);
						break;
					case AVA2_P_POLLING:
					case AVA2_P_DIFF:
					case AVA2_P_REQUIRE:
						send_pkg(AVA2_P_STATUS, (uint8_t *)MM_VERSION, 6);
					default:
						break;
					}
				}
			} else {
				debug32("E: package broken(%02x %02x)\n", pre_last, last);
				send_pkg(AVA2_P_NAK, NULL, 0);
				return 1;
			}
		}

		if (pre_last == AVA2_H1 && last == AVA2_H2 && !start) {
			g_pkg[0] = pre_last;
			g_pkg[1] = last;
			start = 1;
			count = 2;
		}
	}

	return 0;
}

int main(int argv, char **argc) {
	struct mm_work mm_work;
	struct work work;
	struct result result;

	led(0);

	delay(60);		/* Delay 60ms, wait for alink ready */
	adjust_voltage(0x8a00);	/* Configure the power supply for ASICs */

	wdg_init(1);
	wdg_feed((CPU_FREQUENCY / 1000) * 2); /* Configure the wdg to ~2 second, or it will reset FPGA */

	irq_setmask(0);
	irq_enable(1);

	uart_init();
	debug32("MM - %s\n", MM_VERSION);

	alink_init(0x3ff);

	adjust_fan(0);

	g_new_stratum = 0;
	while (1) {
		get_pkg(&mm_work);

		if (!g_new_stratum)
			continue;

		if (alink_txbuf_count() < (24 * 10)) {
			miner_gen_work(&mm_work, &work);
			miner_init_work(&mm_work, &work);
			alink_send_work(&work);

			get_pkg(&mm_work);
			if (!g_new_stratum) {
				alink_flush_fifo();
				continue;
			}
		}

		while (read_result(&result)) {
			get_pkg(&mm_work);
			if (!g_new_stratum) {
				alink_flush_fifo();
				break;
			}
		}

		/* TODO:
		 *   Send out heatbeat information every 2 seconds */

		wdg_feed((CPU_FREQUENCY / 1000) * 2);
	}

	led(0xff);
	return 0;
}
