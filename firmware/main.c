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

static uint8_t g_pkg[AVA4_P_COUNT];
static uint8_t g_act[AVA4_P_COUNT];
static uint8_t g_dna[AVA4_MM_DNA_LEN];
static uint8_t g_led_blinking = 0;
static uint8_t g_postfailed = 0;
static int g_module_id = AVA4_MODULE_BROADCAST;
static int g_new_stratum = 0;
static int g_local_work = 0;
static int g_hw_work = 0;
static uint32_t g_nonce2_offset = 0;
static uint32_t g_nonce2_range = 0xffffffff;
static int g_ntime_offset = ASIC_COUNT;
static struct mm_work mm_work;

#define RET_RINGBUFFER_SIZE_RX 32
#define RET_RINGBUFFER_MASK_RX (RET_RINGBUFFER_SIZE_RX-1)
static uint8_t ret_buf[RET_RINGBUFFER_SIZE_RX][AVA4_P_DATA_LEN];
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

#define LED_OFF_ALL		1
#define LED_WARNING_ON		2
#define LED_WARNING_OFF 	3
#define LED_WARNING_BLINKING	4
#define LED_ERROR_ON		5
#define LED_ERROR_OFF		6
#define LED_ERROR_BLINKING	7
#define LED_IDLE		8
#define LED_BUSY		9
#define LED_PG1_ON		10
#define LED_PG1_BLINKING	11
#define LED_PG2_ON		12
#define LED_PG2_BLINKING	13

static inline void led_ctrl(int led_op)
{
	uint32_t value = get_front_led();

	switch (led_op) {
	case LED_WARNING_ON:
		value &= 0xffffff0f;
		value |= 0x10;
		break;
	case LED_WARNING_OFF:
		value &= 0xffffff0f;
		break;
	case LED_WARNING_BLINKING:
		value &= 0xffffff0f;
		value |= 0x30;
		break;
	case LED_ERROR_ON:
		value &= 0xfffffff0;
		value |= 0x1;
		break;
	case LED_ERROR_OFF:
		value &= 0xfffffff0;
		break;
	case LED_ERROR_BLINKING:
		value &= 0xfffffff0;
		value |= 0x3;
		break;
	case LED_PG1_ON:
		value &= 0xfeffffff;
		value |= 0x1000000;
		break;
	case LED_PG1_BLINKING:
		value &= 0xfeffffff;
		value |= 0x3000000;
		break;
	case LED_PG2_ON:
		value &= 0xefffffff;
		value |= 0x10000000;
		break;
	case LED_PG2_BLINKING:
		value &= 0xefffffff;
		value |= 0x30000000;
		break;
	case LED_BUSY:
		value &= 0xff0000ff;
		value |= 0x444400;
		break;
	case LED_IDLE:
		value &= 0xff0000ff;
		break;
	case LED_OFF_ALL:
		value = 0;
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

	memset(p, 0, AVA4_P_COUNT);

	p[0] = AVA4_H1;
	p[1] = AVA4_H2;

	p[2] = type;
	p[3] = g_dna[AVA4_MM_DNA_LEN-1];
	p[4] = 1;
	p[5] = 1;

	data = p + 6;
	switch(type) {
	case AVA4_P_ACKDETECT:
		p[3] = 0;
		memcpy(data, g_dna, AVA4_MM_DNA_LEN); /* MM_DNA */
		memcpy(data + AVA4_MM_DNA_LEN, buf, len); /* MM_VERSION */
		break;
	case AVA4_P_NONCE:
	case AVA4_P_TEST_RET:
		memcpy(data, buf, len);
		break;
	case AVA4_P_STATUS:
		tmp = read_temp();
		memcpy(data + 0, &tmp, 4);

		tmp = read_fan();
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
	default:
		break;
	}

	crc = crc16(data, AVA4_P_DATA_LEN);
	p[AVA4_P_COUNT - 2] = crc & 0x00ff;
	p[AVA4_P_COUNT - 1] = (crc & 0xff00) >> 8;
}

uint32_t send_pkg(int type, uint8_t *buf, uint32_t len, int block)
{
#ifdef DEBUG_VERBOSE
	debug32("%d-Send: %d, (CNT: %d)\n", g_module_id, type, iic_tx_fifo_cnt());
#endif
	encode_pkg(g_act, type, buf, len);
	if (!iic_write(g_act, AVA4_P_COUNT, block)) {
		iic_tx_reset();
		return 0;
	}

	return len;
}

static inline void polling(void)
{
	uint8_t *data;

	if (ret_consume == ret_produce) {
		send_pkg(AVA4_P_STATUS, NULL, 0, 0);

		g_local_work = 0;
		g_hw_work = 0;
		return;
	}

	data = ret_buf[ret_consume];
	ret_consume = (ret_consume + 1) & RET_RINGBUFFER_MASK_RX;
	send_pkg(AVA4_P_NONCE, data, AVA4_P_DATA_LEN, 0);
	return;
}

static inline int decode_pkg(uint8_t *p, struct mm_work *mw)
{
	static uint32_t freq_value;
	unsigned int expected_crc;
	unsigned int actual_crc;
	int idx, cnt, poweron = 0;
	uint32_t tmp;
	uint32_t val[10], i;
	uint32_t test_core_count;

	uint8_t *data = p + 6;

	idx = p[4];
	cnt = p[5];

#ifdef DEBUG_VERBOSE
	debug32("%d-Decode: %d %d/%d\n", g_module_id, p[2], idx, cnt);
#endif

	expected_crc = (p[AVA4_P_COUNT - 1] & 0xff) |
		((p[AVA4_P_COUNT - 2] & 0xff) << 8);

	actual_crc = crc16(data, AVA4_P_DATA_LEN);
	if(expected_crc != actual_crc) {
		debug32("PKG: CRC failed %d:(W %08x, R %08x)\n",
			p[2], expected_crc, actual_crc);
		return 1;
	}

	timer_set(0, IDLE_TIME);
	switch (p[2]) {
	case AVA4_P_DETECT:
	case AVA4_P_REQUIRE:
		break;

	case AVA4_P_STATIC:
		g_new_stratum = 0;

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
	case AVA4_P_JOB_ID:
		memcpy((uint8_t *)&mw->job_id, data, 4);
		debug32("[%d]J: %08x\n", g_module_id, mw->job_id);
		break;
	case AVA4_P_COINBASE:
		if (idx == 1)
			memset(mw->coinbase, 0, sizeof(mw->coinbase));
		memcpy(mw->coinbase + (idx - 1) * AVA4_P_DATA_LEN, data, AVA4_P_DATA_LEN);
		break;
	case AVA4_P_MERKLES:
		memcpy(mw->merkles[idx - 1], data, AVA4_P_DATA_LEN);
		break;
	case AVA4_P_HEADER:
		memcpy(mw->header + (idx - 1) * AVA4_P_DATA_LEN, data, AVA4_P_DATA_LEN);
		break;
	case AVA4_P_TARGET:
		memcpy(mw->target, data, AVA4_P_DATA_LEN);
		break;
	case AVA4_P_SET:
		/* Chagne voltage and freq in P_SET_VOLT / P_SET_FREQ */
		if (read_temp() >= IDLE_TEMP)
			break;
		memcpy(&tmp, data, 4);
		debug32("N: %08x,", tmp);
		if (tmp & 0x80000000)
			g_ntime_offset = (tmp & 0x7fffffff);

		memcpy(&tmp, data + 4, 4);
		debug32("V: %08x,", tmp);
		poweron = set_voltage(tmp);

		memcpy(&tmp, data + 8, 4);
		if (poweron || tmp != freq_value) {
			freq_value = tmp;

			val[0] = (tmp & 0x3ff00000) >> 20;
			val[1] = (tmp & 0xffc00) >> 10;
			val[2] = tmp & 0x3ff;
			debug32("F: %d|%08x,", poweron, tmp);
			set_asic_freq(val);
		}

		memcpy(&g_nonce2_offset, data + 12, 4);
		memcpy(&g_nonce2_range, data + 16, 4);
		mw->nonce2 = g_nonce2_offset + (g_nonce2_range / AVA4_DEFAULT_MODULES) * g_module_id;
		debug32("[%d] N2: %08x(%08x-%08x)\n", g_module_id, mw->nonce2, g_nonce2_offset, g_nonce2_range);
		break;
	case AVA4_P_SET_VOLT:
		debug32("VOL:");
		for (i = 0; i < 10; i++) {
			val[i] = data[i * 2] << 8 | data[i * 2 + 1];
			debug32(" %08x", val[i]);
		}
		debug32("\n");
		set_voltage_i(val);
		break;
	case AVA4_P_SET_FREQ:
		memcpy(&val[0], data, 4);
		memcpy(&val[1], data + 4, 4);
		memcpy(&val[2], data + 8, 4);
		set_asic_freq_i(val);
		debug32("CPM: %08x-%08x-%08x\n", val[0], val[1], val[2]);
		break;
	case AVA4_P_FINISH:
		if (read_temp() >= IDLE_TEMP)
			break;

		g_new_stratum = 1;
		break;

	case AVA4_P_POLLING:
		polling();

		memcpy(&tmp, data, 4);
		g_led_blinking = tmp;

		memcpy(&tmp, data + 4, 4);
		if (tmp & 0x80000000)
			adjust_fan(tmp & 0x7fffffff);
		break;
	case AVA4_P_TEST:
		adjust_fan(FAN_50);
		wdg_feed(CPU_FREQUENCY * TEST_TIME);

		memcpy(&tmp, data, 4);
		debug32("FULL: %08x", tmp);
		test_core_count = tmp;
		if (!test_core_count)
			test_core_count = TEST_CORE_COUNT;

		memcpy(&tmp, data + 4, 4);
		debug32("V: %08x", tmp);
		set_voltage(tmp);
		for (i = 0; i < 10; i++)
			val[i] = tmp;
		set_voltage_i(val);

		memcpy(&tmp, data + 8, 4);
		val[0] = (tmp & 0x3ff00000) >> 20;
		val[1] = (tmp & 0xffc00) >> 10;
		val[2] = tmp & 0x3ff;
		debug32(" F: %08x(%d:%d:%d)\n", tmp, val[0], val[1], val[2]);
		set_asic_freq(val);

		memcpy(&val[0], data + 12, 4);
		memcpy(&val[1], data + 16, 4);
		memcpy(&val[2], data + 20, 4);
		set_asic_freq_i(val);

		if (api_asic_testcores(test_core_count, 1) < 4 * test_core_count)
			g_postfailed &= 0xfe;
		else
			g_postfailed |= 1;

		set_voltage(ASIC_0V);
		for (i = 0; i < 10; i++)
			val[i] = ASIC_0V;
		set_voltage_i(val);
		adjust_fan(FAN_10);
		wdg_feed(CPU_FREQUENCY * IDLE_TIME);
		break;
	default:
		break;
	}

	return 0;
}

static int read_result(struct mm_work *mw, struct result *ret)
{
	int n, i;
	uint8_t *data, api_ret[4 * LM32_API_RET_LEN];
	uint32_t nonce2, nonce0, memo, job_id, pool_no, miner_id;
	uint32_t ntime = 0, last_nonce0 = 0xbeafbeaf;
	static uint32_t last_minerid = 0xff;
	static uint8_t chip_id;

	if (api_get_rx_cnt() < LM32_API_RET_LEN)
		return 0;

	/* Read result out */
	api_get_rx_fifo((uint32_t *)api_ret);

	memcpy(&nonce0, api_ret + LM32_API_RET_LEN * 4 - 4, 4);
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
	for (i = 0; i < LM32_API_RET_LEN - 3; i++) {
		memcpy(&nonce0, api_ret + 8 + i * 4, 4);
		if (nonce0 == 0xbeafbeaf || nonce0 == last_nonce0)
			continue;

		last_nonce0 = nonce0;

		memcpy(&nonce2, api_ret + 4, 4);
		memcpy(&memo, api_ret, 4);
		job_id = memo & 0xffff0000;
		ntime = (memo & 0xff00) >> 8;

		g_local_work++;
		if (job_id == mw->job_id) {
			n = test_nonce(mw, nonce2, nonce0, ntime);
			if (n == NONCE_HW) {
				g_hw_work++;
				continue;
			}
		}

		if (job_id != mw->job_id || n == NONCE_DIFF) {
			data = ret_buf[ret_produce];
			ret_produce = (ret_produce + 1) & RET_RINGBUFFER_MASK_RX;

			pool_no = memo & 0xff;
			memcpy(ret->pool_no, &pool_no, 4);
			memcpy(ret->nonce2, api_ret + 4, 8);
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

	uint32_t d;

	while (1) {
		if (!iic_read_nonblock() && !start)
			break;

		d = iic_read();
		UNPACK32(d, g_pkg + count * 4);
		if ((uint8_t) ((d) >> 24) == AVA4_H1 &&
		    (uint8_t) ((d) >> 16) == AVA4_H2 && !start) {
			start = 1;
			count = 0;
		}

		count++;

		if (count == AVA4_P_COUNT / 4) {
			if (decode_pkg(g_pkg, mw))
				return 1;

			/* Here we send back PKG if necessary */
			switch (g_pkg[2]) {
			case AVA4_P_DETECT:
				if (g_module_id != AVA4_MODULE_BROADCAST)
					break;

				if (send_pkg(AVA4_P_ACKDETECT, (uint8_t *)MM_VERSION, AVA4_MM_VER_LEN, 1)) {
					memcpy(&g_module_id, g_pkg + 6 + 28, 4);
					debug32("ID: %d\n", g_module_id);
					iic_addr_set(g_module_id);
					gpio_led(g_module_id);
				}
				break;
			case AVA4_P_REQUIRE:
				send_pkg(AVA4_P_STATUS, NULL, 0, 0);
				break;
			default:
				break;
			}

			start = 0;
			count = 0;
		}
	}

	return 0;
}

static inline void led(void)
{
	if (g_new_stratum)
		led_ctrl(LED_BUSY);
	else
		led_ctrl(LED_IDLE);

	/* warn level(high to low) : fan/temp -> stratum */
	if (!read_fan() || read_temp() >= IDLE_TEMP)
		led_ctrl(LED_WARNING_BLINKING);
	else if (g_new_stratum)
		led_ctrl(LED_WARNING_OFF);
	else
		/* g_new_stratum == 0 and timeout */
		if (!timer_read(0))
			led_ctrl(LED_WARNING_ON);

	if (g_led_blinking)
		led_ctrl(LED_ERROR_BLINKING);
	else if (g_postfailed & 1)
		led_ctrl(LED_ERROR_ON);
	else
		led_ctrl(LED_ERROR_OFF);

	if (g_postfailed & 2)
		led_ctrl(LED_PG1_ON);
	else
		led_ctrl(LED_PG1_BLINKING);

	if (g_postfailed & 4)
		led_ctrl(LED_PG2_ON);
	else
		led_ctrl(LED_PG2_BLINKING);
}

#ifdef MBOOT
#include "mboot.c"
#endif

int main(int argv, char **argc)
{
	struct work work;
	struct result result;
	int i;
	uint32_t val[10];

#ifdef MBOOT
	mboot();
#endif

	adjust_fan(FAN_10);

	irq_setmask(0);
	irq_enable(1);

	iic_init();
	iic_addr_set(g_module_id);
	gpio_led(g_module_id);

	api_initial(MINER_COUNT, ASIC_COUNT, SPI_SPEED);

	debug32("%d:%s\n", g_module_id, MM_VERSION);

	/* Dump the FPGA DNA */
	iic_dna_read(g_dna);
	hexdump(g_dna, AVA4_MM_DNA_LEN);
#ifdef DEBUG_IIC_TEST
	extern void iic_test(void);
	iic_test();
#endif
	timer_set(0, IDLE_TIME);
	timer_set(1, 0);
	led_ctrl(LED_OFF_ALL);

#if 1
	/* Test part of ASIC cores */
	uint32_t freq[3] = {200, 200, 200};
	uint32_t cpm[3] = {0x1e0784c7, 0x1e0784c7, 0x1e0784c7};

	set_voltage(ASIC_CORETEST_VOLT);
	for (i = 0; i < 10; i++) {
		val[i] = ASIC_CORETEST_VOLT;
	}
	set_voltage_i(val);
	set_asic_freq(freq);
	set_asic_freq_i(cpm);
	if (api_asic_testcores(TEST_CORE_COUNT, 0) >= 4 * TEST_CORE_COUNT)
		g_postfailed |= 1;
#endif

	i = read_power_good();
	if (i == 0x1f || i == 0x3e0)
		g_postfailed |= 1;

	if ((i & 0x1f) == 0x1f)
		g_postfailed |= 2;
	else
		g_postfailed &= 0xfd;

	if (((i >> 5) & 0x1f) == 0x1f)
		g_postfailed |= 4;
	else
		g_postfailed &= 0xfb;

	set_voltage(ASIC_0V);
	for (i = 0; i < 10; i++) {
		val[i] = ASIC_0V;
	}
	set_voltage_i(val);
	g_new_stratum = 0;
	while (1) {
		wdg_feed(CPU_FREQUENCY * IDLE_TIME);

		get_pkg(&mm_work);

		if ((!timer_read(0) && (g_new_stratum || g_module_id)) ||
		    read_temp() >= IDLE_TEMP) {
			g_new_stratum = 0;
			g_local_work = 0;
			g_hw_work = 0;
			g_ntime_offset = ASIC_COUNT;

			set_voltage(ASIC_0V);
			for (i = 0; i < 10; i++) {
				val[i] = ASIC_0V;
			}
			set_voltage_i(val);

			if (read_temp() >= IDLE_TEMP) {
				adjust_fan(FAN_100);
			} else {
				adjust_fan(FAN_10);

				g_nonce2_offset = 0;
				g_nonce2_range = 0xffffffff;
				g_module_id = AVA4_MODULE_BROADCAST;

				iic_addr_set(g_module_id);
				iic_rx_reset();
				iic_tx_reset();

				ret_consume = ret_produce;

				gpio_led(g_module_id);
			}
		}

		led();

		if (!g_new_stratum)
			continue;

		if (api_get_tx_cnt() <= 23 * 10) {
			miner_gen_nonce2_work(&mm_work, mm_work.nonce2++, &work);
			api_send_work(&work);

			for (i = 1; i < g_ntime_offset; i++) {
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
