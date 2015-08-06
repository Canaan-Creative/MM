/*
 * Author: Xiangfu Liu <xiangfu@openmobilefree.net>
 * Bitcoin:	1CanaaniJzgps8EV6Sfmpb7T8RutpaeyFn
 *
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */

#include <stdint.h>
#include <stddef.h>

#include "minilibc/minilibc.h"
#include "system_config.h"
#include "defines.h"
#include "miner.h"
#include "io.h"
#include "api.h"

static struct lm32_api *api = (struct lm32_api *)API_BASE;

static uint32_t g_asic_freq[3] = {200, 200, 200};
static uint32_t g_asic_freq_avg = 0;

static inline void api_set_num(uint32_t ch_num, uint32_t chip_num)
{
	uint32_t tmp;
	tmp = readl(&api->sck) & 0xff;
	tmp = (ch_num << 16) | ((chip_num*23) << 23) | tmp;
	writel(tmp, &api->sck);
}

static inline void api_set_sck(uint32_t spi_speed)
{
	uint32_t tmp;
	tmp = (readl(&api->sck) & 0xffff0000) | spi_speed;
	writel(tmp, &api->sck);
}

static void api_set_tx_fifo(uint32_t * data)
{
	int i;
	for (i = 0; i < 23; i++)
		writel(data[i], &api->tx);
}

static inline void api_wait_done(uint32_t ch_num, uint32_t chip_num)
{
	while (api_get_rx_cnt() != (ch_num * chip_num * LM32_API_RET_LEN))
		;
}

static inline uint32_t api_gen_test_work(uint32_t i, uint32_t *data)
{
	uint32_t tmp;
	int j;

	for (j = 0; j < 18; j++){
		writel((i%16) * 18 + j, &api->ram);
		data[j] = readl(&api->ram);
	}

	tmp = data[0];
        data[0] = data[0] ^ (i & 0xfffffff0);	/* nonce */

	data[18] = 0x0;	/* nonce2 */
	data[19] = i + 1;	/* nonce2 */
	data[20] = 0x1;	/* cpm2 */
	data[21] = 0x1;	/* cpm1 */
	data[22] = 0x1;	/* cpm0 */

	return tmp + 0x18000;
}

static uint32_t api_verify_nonce(uint32_t ch_num, uint32_t chip_num,
				 uint32_t verify_on, uint32_t target_nonce,
				 uint32_t result[MINER_COUNT][ASIC_COUNT])
{
	static uint8_t chip_id = 0;

	uint32_t i, j;
	uint32_t rx_data[LM32_API_RET_LEN];
	uint32_t pass_cal_num = 0;
	uint8_t channel_id;

	for (i = 0; i < ch_num; i++) {
		for (j = 0; j < chip_num; j++) {
			api_get_rx_fifo(rx_data);
			channel_id = rx_data[10] & 0xff;

			if (verify_on && ((rx_data[2] == target_nonce) || (rx_data[3] == target_nonce) || (rx_data[4] == target_nonce))) {
				pass_cal_num++;
			} else {
#ifdef DEBUG_VERBOSE
				if (result)
					debug32("channel id: %d,chip id: %d, TN:%08x, RX[0]:%08x, RX[1]:%08x, RX[2]:%08x, RX[3]:%08x\n", channel_id, chip_id, target_nonce, rx_data[0], rx_data[1], rx_data[2], rx_data[3]);
#endif
				if (verify_on && result)
				    result[channel_id][chip_id]++;
			}

			if (ASIC_COUNT > 1) {
			    chip_id++;
			    chip_id %= ASIC_COUNT;
			}
		}
	}

	return pass_cal_num;
}

static inline void api_flush(void)
{
	writel(0x2, &api->state);
	delay(1);
}

static void api_change_cpm(uint32_t ch_num, uint32_t chip_num, uint32_t cpm1, uint32_t cpm2, uint32_t cpm3)
{
#ifdef MM50
	uint32_t tx_data[23];
	uint32_t i, j;

	/* random numbers, keep different with nonce2 */
	tx_data[18] = 0xa8bc6de9;
	tx_data[19] = 0x35416784;
	tx_data[20] = cpm1;
	tx_data[21] = cpm2;
	tx_data[22] = cpm3;

	for (i = 0; i < ch_num; i++) {
		for (j = 0; j < chip_num; j++)
			api_set_tx_fifo(tx_data);
	}

	api_wait_done(ch_num, chip_num);

	api_verify_nonce(ch_num, chip_num, 0, 0, NULL);
	delay(1);
#else
	uint32_t tx_data[23];
	uint32_t i, k, j;

	for (j = 0; j < 3; j++) {
		/* random numbers, keep different with nonce2 */
		tx_data[18] = 0xa8bc6de9 + j;
		tx_data[19] = 0x35416784 + j;
		if (j == 0) {
			tx_data[20] = cpm1;
			tx_data[21] = 1;
			tx_data[22] = 1;
		} else if (j == 1) {
			tx_data[20] = 1;
			tx_data[21] = cpm2;
			tx_data[22] = 1;
		} else if (j == 2) {
			tx_data[20] = 1;
			tx_data[21] = 1;
			tx_data[22] = cpm3;
		}

		for (k = 0; k < ch_num; k++) {
			for(i = 0; i < chip_num; i++)
				api_set_tx_fifo(tx_data);
		}

		api_wait_done(ch_num, chip_num);

		api_verify_nonce(ch_num, chip_num, 0, 0, NULL);
		delay(1);
	}
#endif
}

void api_initial(uint32_t ch_num, uint32_t chip_num, uint32_t spi_speed)
{
	api_set_num(ch_num, chip_num);
	api_set_sck(spi_speed);
}

void api_set_timeout(uint32_t timeout)
{
	writel(timeout, &api->timeout);
}

uint32_t api_get_tx_cnt(void)
{
	return (readl(&api->state) >> 2) & 0x3ff;
}

uint32_t api_get_rx_cnt(void)
{
	return (readl(&api->state) >> 20) & 0x3ff;
}

void api_get_rx_fifo(uint32_t * data)
{
	int i;
	for (i = 0; i < LM32_API_RET_LEN; i++)
		data[i] = readl(&api->rx);
}

static uint32_t api_asic_test(uint32_t ch_num, uint32_t chip_num,
			      uint32_t cal_core_num, uint32_t add_step,
			      uint32_t result[MINER_COUNT][ASIC_COUNT])
{
	uint32_t i, j, k;
	uint32_t tx_data[23];
	uint32_t target_nonce;
	uint32_t pass_cal_num = 0;
	uint32_t verify_on = 0;
	uint32_t tmp;
	api_set_timeout(0x10);
	api_flush();

	if (result)
		memset(result, 0, MINER_COUNT * ASIC_COUNT * sizeof(uint32_t));

	for (j = 0; j < cal_core_num + 2 * add_step; j += add_step) {
		api_gen_test_work(j, tx_data);
		for (k = 0; k < ch_num; k++) {
			for (i = 0; i < chip_num; i++)
				api_set_tx_fifo(tx_data);
		}

		api_wait_done(ch_num, chip_num);

		target_nonce = api_gen_test_work((j - 2 * add_step) % 16, tx_data);
		verify_on = (j >= 2 ? 1 : 0);
		tmp = api_verify_nonce(ch_num, chip_num, verify_on, target_nonce, result);
		pass_cal_num += tmp;

#ifdef DEBUG_VERBOSE
		uint32_t pass_zone_num[3] = {0, 0, 0};

		if (verify_on) {
			if ((j - 2) < (28 * 4 - 4) * 16)
				pass_zone_num[0] += tmp;
			else if ((j - 2) < (28 * 4 - 4 + 28 * 4) * 16)
				pass_zone_num[1] += tmp;
			else
				pass_zone_num[2] += tmp;
		}
#endif
	}

	return pass_cal_num;
}

int api_send_work(struct work *w)
{
#if 0
	int i, j;

	writel(0, &api->tx);

	i = 0;

	for (j = 1; j < 18; j++)
		writel(test_data[i%16][j], &api->tx);
#else
	uint32_t tmp;
	int i;

	writel(0, &api->tx);

	memcpy((uint8_t *)(&tmp), w->a2, 4);
	writel(tmp, &api->tx);

	for (i = 0; i <= 28; i += 4) {
		memcpy((uint8_t *)(&tmp), w->data + i, 4);
		writel(tmp, &api->tx);
	}

	memcpy((uint8_t *)(&tmp), w->e0, 4);
	writel(tmp, &api->tx);
	memcpy((uint8_t *)(&tmp), w->e1, 4);
	writel(tmp, &api->tx);
	memcpy((uint8_t *)(&tmp), w->e2, 4);
	writel(tmp, &api->tx);
	memcpy((uint8_t *)(&tmp), w->a0, 4);
	writel(tmp, &api->tx);
	memcpy((uint8_t *)(&tmp), w->a1, 4);
	writel(tmp, &api->tx);

	for (i = 0; i <= 8; i += 4) {
		memcpy((uint8_t *)(&tmp), w->data + 32 + i, 4);
		writel(tmp, &api->tx);
	}

	memcpy((uint8_t *)(&tmp), &w->memo, 4);
	writel(tmp, &api->tx);

	memcpy((uint8_t *)(&tmp), &w->nonce2, 4);
	writel(tmp, &api->tx);
#endif

	/* The chip configure information */
	writel(1, &api->tx);
	writel(1, &api->tx);
	writel(1, &api->tx);

	return 0;
}

static inline void set_asic_timeout(uint32_t value[])
{
	int i;
	uint32_t max_freq = 0;

	for (i = 0; i < 3; i++)
		g_asic_freq[i] = value[i];

	max_freq = g_asic_freq[0];
	for (i = 1; i < 3; i++) {
		if (max_freq < g_asic_freq[i])
			max_freq = g_asic_freq[i];
	}

	/* The timeout value:
	 * 2^32÷(0.1GHz×1000000000×3968÷65)×100000000 = 0x4318c63 */
	api_set_timeout((ASIC_TIMEOUT_100M / max_freq) * 100 / 28);
	api_flush();

	g_asic_freq_avg = (g_asic_freq[0] + g_asic_freq[1] * 4 + g_asic_freq[2] * 4) / 9;
}

/* 1 -> 4 -> 4 */
void set_asic_freq(uint32_t value[])
{
	set_asic_timeout(value);
}

/* Must call set_asic_freq first, Call from AVA4_P_SET_FREQ */
void set_asic_freq_i(uint32_t cpm[])
{
	api_change_cpm(MINER_COUNT, ASIC_COUNT, cpm[0], cpm[1], cpm[2]);
}

uint32_t get_asic_freq(void)
{
	return g_asic_freq_avg;
}

int api_asic_testcores(uint32_t cal_core_num, uint32_t ret)
{
	int i = 0, j = 0;
	uint8_t txdat[4 * 4 + 1];
	uint32_t result[MINER_COUNT][ASIC_COUNT];
	uint32_t tmp;
	uint32_t all = MINER_COUNT * ASIC_COUNT * cal_core_num;

	tmp = api_asic_test(MINER_COUNT, ASIC_COUNT, cal_core_num, 1, result);

	for (i = 0; i < MINER_COUNT; i++) {
		txdat[0] = i;
		debug32("%d: ", i);
		for (j = 0; j < ASIC_COUNT; j++) {
			txdat[1 + (j % 4) * 4] = (result[i][j] >> 24) & 0xff;
			txdat[2 + (j % 4) * 4] = (result[i][j] >> 16) & 0xff;
			txdat[3 + (j % 4) * 4] = (result[i][j] >> 8) & 0xff;
			txdat[4 + (j % 4) * 4] = result[i][j] & 0xff;
			debug32("%d ", result[i][j]);

			if (ret && !((j + 1) % 4))
				send_pkg(AVA4_P_TEST_RET, txdat, 4 * 4 + 1, 0);

		}
		debug32("\n");
	}
	if (ret) {
		txdat[0] = (tmp >> 24) & 0xff;
		txdat[1] = (tmp >> 16) & 0xff;
		txdat[2] = (tmp >> 8) & 0xff;
		txdat[3] = tmp & 0xff;
		txdat[4] = (all >> 24) & 0xff;
		txdat[5] = (all >> 16) & 0xff;
		txdat[6] = (all >> 8) & 0xff;
		txdat[7] = all & 0xff;
		send_pkg(AVA4_P_TEST_RET, txdat, 8, 0);
	}

	debug32("E/A: %d/%d\n", all - tmp, all);
	return all - tmp;
}

void api_get_lw(uint32_t *buf)
{
	int i;

	for (i = 0; i < MINER_COUNT; i++) {
		writel(i, &api->lw);
		buf[i] = readl(&api->lw);
	}
}

void api_set_pll(uint32_t miner_id, uint32_t chip_id, uint32_t pll_data0, uint32_t pll_data1, uint32_t pll_data2)
{
	uint32_t tmp = 0;
	tmp = (miner_id % MINER_COUNT) | ((chip_id % ASIC_COUNT) << 4);
	writel(tmp, &api->plla);
	writel(pll_data0, &api->pllc);
	writel(pll_data1, &api->pllc);
	writel(pll_data2, &api->pllc);
}

uint32_t api_get_pll_fifo_count(void)
{
	uint32_t tmp;
	tmp = readl(&api->plla);
	tmp = (tmp >> 12) & 0x7f;
	return tmp;
}

uint32_t api_get_pll_fifo_full(void)
{
	uint32_t tmp;
	tmp = readl(&api->plla);
	tmp = (tmp >> 21) & 0x1;
	return tmp;
}

uint32_t api_get_pll_fifo_empty(void)
{
	uint32_t tmp;
	tmp = readl(&api->plla);
	tmp = (tmp >> 22) & 0x1;
	return tmp;
}

void api_get_pll_fifo_reset(void)
{
	writel(0x1 << 23, &api->plla);
}
