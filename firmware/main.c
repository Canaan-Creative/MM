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
#include "iic.h"
#include "miner.h"
#include "sha256.h"
#include "twipwm.h"
#include "shifter.h"
#include "timer.h"
#include "protocol.h"
#include "crc.h"
#include "api.h"

#include "hexdump.c"

#define IDLE_TIME	5	/* Seconds */
#define IDLE_TEMP	90	/* Degree (C) */

static uint8_t g_pkg[AVA2_P_COUNT+1];
static uint8_t g_act[AVA2_P_COUNT+1];
static uint8_t g_dna[8];
static int g_module_id = AVA2_MODULE_BROADCAST;
static int g_new_stratum = 0;
static int g_local_work = 0;
static int g_hw_work = 0;
static uint32_t g_nonce2_offset = 0;
static uint32_t g_nonce2_range = 0xffffffff;
static struct mm_work mm_work;

#define RET_RINGBUFFER_SIZE_RX 32
#define RET_RINGBUFFER_MASK_RX (RET_RINGBUFFER_SIZE_RX-1)
static uint8_t ret_buf[RET_RINGBUFFER_SIZE_RX][AVA2_P_DATA_LEN];
static volatile unsigned int ret_produce = 0;
static volatile unsigned int ret_consume = 0;

#define UNPACK32(x, str)			\
{						\
	*((str) + 3) = (uint8_t) ((x)      );	\
	*((str) + 2) = (uint8_t) ((x) >>  8);	\
	*((str) + 1) = (uint8_t) ((x) >> 16);	\
	*((str) + 0) = (uint8_t) ((x) >> 24);	\
}

void delay(unsigned int ms)
{
	unsigned int i;

	while (ms && ms--) {
		for (i = 0; i < CPU_FREQUENCY / 1000 / 5; i++)
			__asm__ __volatile__("nop");
	}
}

#define LED_OFF_ALL	0
#define LED_ON_ALL	1
#define LED_IDLE	2
#define LED_BUSY	3
#define LED_POSTON	4
#define LED_POSTOFF	5
#define LED_POWER	6

static inline void led_ctrl(int led_op)
{
	uint32_t value = get_front_led();

	switch (led_op) {
	case LED_OFF_ALL:
		value = 0;
		break;
	case LED_ON_ALL:
		value = 0x11111111;
		break;
	case LED_IDLE:
		value &= 0xffffff0f;
		value |= 0x30;
		break;
	case LED_BUSY:
		value &= 0xff0000ff;
		value |= 0x444400;
		break;
	case LED_POSTON:
		value &= 0xfffffff0;
		value |= 0x1;
		break;
	case LED_POSTOFF:
		value &= 0xfffffff0;
		break;
	case LED_POWER:
		value &= 0xfffffff;
		value |= 0x10000000;
		break;
	default:
		return;
	}

	set_front_led(value);
}

static void encode_pkg(uint8_t *p, int type, uint8_t *buf, unsigned int len)
{
	uint32_t tmp;
	uint16_t crc;
	uint8_t *data;

	memset(p, 0, AVA2_P_COUNT + 1);

	p[0] = AVA2_H1;
	p[1] = AVA2_H2;

	p[2] = type;
	p[3] = 1;
	p[4] = 1;

	data = p + 5;
	memcpy(data + 28, &g_module_id, 4); /* Attach the module_id at end */

	switch(type) {
	case AVA2_P_ACKDETECT:
		memcpy(data, buf, len);
		memcpy(data + len, g_dna, 8);
		break;
	case AVA2_P_NONCE:
	case AVA2_P_TEST_RET:
		memcpy(data, buf, len);
		break;
	case AVA2_P_STATUS:
		tmp = read_temp();
		tmp |= tmp << 16;
		memcpy(data + 0, &tmp, 4);

		tmp = read_fan();
		tmp |= tmp << 16;
		memcpy(data + 4, &tmp, 4);

		tmp = get_asic_freq();
		memcpy(data + 8, &tmp, 4);
		tmp = get_voltage();
		memcpy(data + 12, &tmp, 4);

		memcpy(data + 16, &g_local_work, 4);
		memcpy(data + 20, &g_hw_work, 4);

		tmp = read_power_good();
		memcpy(data + 24, &tmp, 4);
		break;
	case AVA2_P_ACKDISCOVER:
		memcpy(data, g_dna, 8); /* MM_DNA */
		memcpy(data + 8, buf, len); /* MM_VERSION */
		break;
	default:
		break;
	}

	crc = crc16(data, AVA2_P_DATA_LEN);
	p[AVA2_P_COUNT - 2] = crc & 0x00ff;
	p[AVA2_P_COUNT - 1] = (crc & 0xff00) >> 8;
}

uint32_t send_pkg(int type, uint8_t *buf, uint32_t len, int block)
{
#if DEBUG_VERBOSE
	debug32("%d-Send: %d\n", g_module_id, type);
#endif
	encode_pkg(g_act, type, buf, len);
	if (!iic_write(g_act, AVA2_P_COUNT + 1, block)) {
		iic_tx_reset();
		return 0;
	}

	return len;
}

static void polling()
{
	uint8_t *data;

	if (ret_consume == ret_produce) {
		send_pkg(AVA2_P_STATUS, NULL, 0, 0);

		g_local_work = 0;
		g_hw_work = 0;
		return;
	}

	data = ret_buf[ret_consume];
	ret_consume = (ret_consume + 1) & RET_RINGBUFFER_MASK_RX;
	send_pkg(AVA2_P_NONCE, data, AVA2_P_DATA_LEN - 4, 0);
	return;
}

static int decode_pkg(uint8_t *p, struct mm_work *mw)
{
	unsigned int expected_crc;
	unsigned int actual_crc;
	int idx, cnt;
	uint32_t tmp;
	uint32_t freq[3];

	uint8_t *data = p + 5;

	idx = p[3];
	cnt = p[4];

#if DEBUG_VERBOSE
	debug32("%d-Decode: %d %d/%d\n", g_module_id, p[2], idx, cnt);
#endif
	expected_crc = (p[AVA2_P_COUNT - 1] & 0xff) |
		((p[AVA2_P_COUNT - 2] & 0xff) << 8);

	actual_crc = crc16(data, AVA2_P_DATA_LEN);
	if(expected_crc != actual_crc) {
		debug32("PKG: CRC failed %d:(W %08x, R %08x)\n",
			p[2], expected_crc, actual_crc);
		return 1;
	}

	timer_set(0, IDLE_TIME);
	switch (p[2]) {
	case AVA2_P_DETECT:
		g_new_stratum = 0;
		g_local_work = 0;
		g_hw_work = 0;
		/* TODO: Flash api fifo ?? */
		gpio_led(0);
		break;
	case AVA2_P_STATIC:
		g_new_stratum = 0;
		g_local_work = 0;
		g_hw_work = 0;
		/* TODO: Flash api fifo ?? */
		memcpy(&mw->coinbase_len, data, 4);
		memcpy(&mw->nonce2_offset, data + 4, 4);
		memcpy(&mw->nonce2_size, data + 8, 4);
		memcpy(&mw->merkle_offset, data + 12, 4);
		memcpy(&mw->nmerkles, data + 16, 4);
		memcpy(&mw->diff, data + 20, 4);
		memcpy(&mw->pool_no, data + 24, 4);
		debug32("S: (%d) %d, %d, %d, %d, %d, %d\n",
			mw->pool_no,
			mw->coinbase_len,
			mw->nonce2_offset,
			mw->nonce2_size,
			mw->merkle_offset,
			mw->nmerkles,
			mw->diff);
		break;
	case AVA2_P_JOB_ID:
		memcpy((uint8_t *)&mw->job_id, data, 4);
		debug32("[%d]J: %08x\n", g_module_id, mw->job_id);
		break;
	case AVA2_P_COINBASE:
		if (idx == 1)
			memset(mw->coinbase, 0, sizeof(mw->coinbase));
		memcpy(mw->coinbase + (idx - 1) * AVA2_P_DATA_LEN, data, AVA2_P_DATA_LEN);
		break;
	case AVA2_P_MERKLES:
		memcpy(mw->merkles[idx - 1], data, AVA2_P_DATA_LEN);
		break;
	case AVA2_P_HEADER:
		memcpy(mw->header + (idx - 1) * AVA2_P_DATA_LEN, data, AVA2_P_DATA_LEN);
		break;
	case AVA2_P_POLLING:
		memcpy(&tmp, data + 28, 4);
#if DEBUG_VERBOSE
		debug32("ID: %d-%d\n", g_module_id, tmp);
#endif
		if (g_module_id != tmp)
			break;

		polling();

		memcpy(&tmp, data + 24, 4);
		if (tmp) {
			memcpy(&tmp, data, 4);
			adjust_fan(tmp);
			memcpy(&tmp, data + 4, 4);
			if (set_voltage(tmp)) {
				memcpy(&tmp, data + 8, 4);
				freq[0] = tmp;
				freq[1] = tmp;
				freq[2] = tmp;
				set_asic_freq(freq);
			}
		}

		memcpy(&tmp, data + 12, 4);
		gpio_led(tmp);
		break;
	case AVA2_P_REQUIRE:
		break;
	case AVA2_P_SET:
		if (read_temp() >= IDLE_TEMP)
			break;

		memcpy(&tmp, data, 4);
#ifdef DEBUG_VERBOSE
		debug32("F: %08x", tmp);
#endif
		adjust_fan(tmp);
		memcpy(&tmp, data + 4, 4);
#ifdef DEBUG_VERBOSE
		debug32(" V: %08x", tmp);
#endif
		if (set_voltage(tmp)) {
			memcpy(&tmp, data + 8, 4);
			freq[0] = (tmp & 0x3ff00000) >> 20;
			freq[1] = (tmp & 0xffc00) >> 10;
			freq[2] = tmp & 0x3ff;
#ifdef DEBUG_VERBOSE
			debug32(" F: %08x", tmp);
#endif
			set_asic_freq(freq);
		}

		memcpy(&g_nonce2_offset, data + 12, 4);
		memcpy(&g_nonce2_range, data + 16, 4);
		mw->nonce2 = g_nonce2_offset + (g_nonce2_range / AVA4_DEFAULT_MODULES) * g_module_id;
#ifdef DEBUG_VERBOSE
		debug32(" N2: %08x(%08x-%08x|%d)\n", mw->nonce2, g_nonce2_offset, g_nonce2_range, g_module_id);
#endif
		g_new_stratum = 1;
		break;
	case AVA2_P_TARGET:
		memcpy(mw->target, data, AVA2_P_DATA_LEN);
		break;
	case AVA2_P_TEST:
		memcpy(&tmp, data + 28, 4);
		if (g_module_id != tmp)
			break;

		gpio_led(1);
		led_ctrl(LED_POSTON);
		memcpy(&tmp, data + 4, 4);
		debug32("V: %08x", tmp);
		set_voltage(tmp);
		memcpy(&tmp, data + 8, 4);
		freq[0] = (tmp & 0x3ff00000) >> 20;
		freq[1] = (tmp & 0xffc00) >> 10;
		freq[2] = tmp & 0x3ff;
		debug32(", F: %08x/%d-%d-%d\n", tmp, freq[0], freq[1], freq[2]);
		{
			int m = MINER_COUNT;
			int c = ASIC_COUNT;
			int all = m * c;
			uint8_t result[8];

			tmp = api_asic_test(m, c, all/m/c, 1, NULL, freq);
			result[0] = (tmp >> 24) & 0xff;
			result[1] = (tmp >> 16) & 0xff;
			result[2] = (tmp >> 8) & 0xff;
			result[3] = tmp & 0xff;
			result[4] = (all >> 24) & 0xff;
			result[5] = (all >> 16) & 0xff;
			result[6] = (all >> 8) & 0xff;
			result[7] = all & 0xff;
			debug32("A.T: pass %d, all %d\n", tmp, all);
			send_pkg(AVA2_P_TEST_RET, result, 8, 0);
		}

		set_voltage(ASIC_0V);
		gpio_led(0);
		led_ctrl(LED_POSTOFF);
		break;
	default:
		break;
	}

	return 0;
}

static int read_result(struct mm_work *mw, struct result *ret)
{
	int n, i;
	uint8_t *data, api_ret[4 * LM32_API_RX_BUFF_LEN];
	uint32_t nonce2, nonce0, memo, job_id, pool_no, miner_id;
	uint32_t ntime = 0, last_nonce0 = 0xbeafbeaf;
	static uint32_t last_minerid = 0xff;
	static uint8_t chip_id;

	if (api_get_rx_cnt() < LM32_API_RX_BUFF_LEN)
		return 0;

	/* Read result out */
	api_get_rx_fifo((unsigned int *)api_ret);

	memcpy(&nonce0, api_ret + LM32_API_RX_BUFF_LEN * 4 - 4, 4);
	if ((nonce0 & 0xffffff00) != 0xbeaf1200)
		return 1;

	miner_id = nonce0 & 0xff;

	/* Calculate chip id */
	if (last_minerid != miner_id) {
		chip_id = 0;
		last_minerid = miner_id;
	} else
		chip_id++;

	/* Handle the real nonce */
	for (i = 0; i < LM32_API_RX_BUFF_LEN - 3; i++) {
		memcpy(&nonce0, api_ret + 8 + i * 4, 4);
		if (nonce0 == 0xbeafbeaf || nonce0 == last_nonce0)
			continue;

		last_nonce0 = nonce0;
		g_local_work++;

		memcpy(&nonce2, api_ret + 4, 4);
		memcpy(&memo, api_ret, 4);
		job_id = memo & 0xffff0000;
		ntime = (memo & 0xff00) >> 8;

		n = test_nonce(mw, nonce2, nonce0, ntime);
		if (n == NONCE_HW && job_id == mw->job_id) {
			g_hw_work++;
			continue;
		}

		if (n == NONCE_DIFF || job_id != mw->job_id) {
			data = ret_buf[ret_produce];
			ret_produce = (ret_produce + 1) & RET_RINGBUFFER_MASK_RX;

			pool_no = memo & 0xff;
			memcpy(ret->pool_no, &pool_no, 4);
			memcpy(ret->nonce2, api_ret + 4, 8);
			nonce0 = nonce0 - 0x4000 + 0x180;
			memcpy(ret->nonce, &nonce0, 4);
			memcpy(ret->ntime, &ntime, 4);
			miner_id |= chip_id << 16;
			memcpy(ret->miner_id, &miner_id, 4);

			memcpy(data, (uint8_t *)ret, 20);
			memcpy(data + 20, &job_id, 4); /* Attach the job_id */
		}
	}

	return 1;
}

static int get_pkg(struct mm_work *mw)
{
	static int start = 0, count = 0;

	uint32_t d, tmp;

	while (1) {
		if (!iic_read_nonblock() && !start)
			break;

		d = iic_read();
		UNPACK32(d, g_pkg + count * 4);
		if ((uint8_t) ((d) >> 24) == AVA2_H1 &&
		    (uint8_t) ((d) >> 16) == AVA2_H2 && !start) {
			start = 1;
			count = 0;
		}

		count++;

		if (count == (AVA2_P_COUNT + 1) / 4 ) {
			start = 0;
			count = 0;

			if (decode_pkg(g_pkg, mw)) {
#ifdef CFG_ENABLE_ACK
				send_pkg(AVA2_P_NAK, NULL, 0, 0);
#endif
				return 1;
			} else {
				/* Here we send back PKG if necessary */
#ifdef CFG_ENABLE_ACK
				send_pkg(AVA2_P_ACK, NULL, 0, 0);
#endif
				switch (g_pkg[2]) {
				case AVA2_P_DETECT:
					memcpy(&tmp, g_pkg + 5 + 28, 4);
					if (g_module_id == tmp)
						send_pkg(AVA2_P_ACKDETECT, (uint8_t *)MM_VERSION, MM_VERSION_LEN, 1);
					break;
				case AVA2_P_REQUIRE:
					memcpy(&tmp, g_pkg + 5 + 28, 4);
					if (g_module_id == tmp)
						send_pkg(AVA2_P_STATUS, NULL, 0, 0);
					break;
				case AVA2_P_DISCOVER:
					if (g_module_id == AVA2_MODULE_BROADCAST)
						if (send_pkg(AVA2_P_ACKDISCOVER, (uint8_t *)MM_VERSION, MM_VERSION_LEN, 1)) {
							memcpy(&g_module_id, g_pkg + 5 + 28, 4);
							debug32("ID: %d\n", g_module_id);
							iic_addr_set(g_module_id);
						}
					break;
				default:
					break;
				}
			}
		}
	}

	return 0;
}

int main(int argv, char **argc)
{
	struct work work;
	struct result result;
	int i;

	adjust_fan(0x3ff);		/* Set the fan to 100% */
	led_ctrl(LED_ON_ALL);
	delay(2000);
	led_ctrl(LED_OFF_ALL);
	adjust_fan(0x2ff);		/* Set the fan to 50% */

	/* TODO: Flash api fifo */

	wdg_init(1);
	wdg_feed((CPU_FREQUENCY / 1000) * 2); /* Configure the wdg to ~2 second, or it will reset FPGA */

	irq_setmask(0);
	irq_enable(1);

	iic_init();
	iic_addr_set(g_module_id);

	/* Dump the FPGA DNA */
	iic_dna_read(g_dna);
	hexdump(g_dna, 8);
	debug32("%d:MM-%s,%dC\n", g_module_id, MM_VERSION, read_temp());
#ifdef DEBUG_IIC_TEST
	extern void iic_test(void);
	iic_test();
#endif
	api_initial(MINER_COUNT, ASIC_COUNT, SPI_SPEED);

	timer_set(0, IDLE_TIME);
	gpio_led(0xf);
	led_ctrl(LED_POWER);

#if 0
	if (1) {
		/* Test part of ASIC cores */
		set_voltage(ASIC_CORETEST_VOLT);
		int ret;
		uint32_t freq[3];
		int m = MINER_COUNT;
		int c = ASIC_COUNT;
		int all = m * c * (248 * 16);

		unsigned int add_step = 1;
		unsigned int pass_zone_num[3];

		adjust_fan(0);

		freq[0] = freq[1] = freq[2] = 200;
		ret = api_asic_test(m, c, all/m/c, add_step, pass_zone_num, freq);
		debug32("DEBUG pass_zone_num %d/%d, %d/%d, %d/%d\n",
				pass_zone_num[0], m*c*(28*4-4)*16,
				pass_zone_num[1], m*c*(28*4)*16,
				pass_zone_num[2], m*c*28*16);

		debug32("A.T: %d / %d = %d%%\n", all-ret, all, ((all-ret)*100/all));
		adjust_fan(0x2ff);
		set_voltage(ASIC_0V);
	}
#endif

	led_ctrl(LED_IDLE);
	set_voltage(ASIC_0V);
	g_new_stratum = 0;
	while (1) {
		get_pkg(&mm_work);

		wdg_feed((CPU_FREQUENCY / 1000) * 2);
		if ((!timer_read(0) && g_new_stratum) ||
		    read_temp() >= IDLE_TEMP) {
			g_new_stratum = 0;
			g_local_work = 0;
			g_hw_work = 0;

			adjust_fan(0x2ff);
			set_voltage(ASIC_0V);

			iic_rx_reset();
			iic_tx_reset();
			led_ctrl(LED_IDLE);
		}

		if (!g_new_stratum)
			continue;

		led_ctrl(LED_BUSY);
		if (api_get_tx_cnt() <= (23 * 8)) {
			miner_gen_nonce2_work(&mm_work, mm_work.nonce2++, &work);
			api_send_work(&work);

			for (i = 1; i < ASIC_COUNT; i++) {
				roll_work(&work, 1); /* Roll the same work */
				work.memo &= 0xffff00ff;
				work.memo |= i << 8;

				api_send_work(&work);
			}
		}
		read_result(&mm_work, &result);
	}

	return 0;
}
