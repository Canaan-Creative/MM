/*
 * Author: Xiangfu Liu <xiangfu@openmobilefree.net>
 * Bitcoin:	1CanaaniJzgps8EV6Sfmpb7T8RutpaeyFn
 *
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */

#include <stdint.h>

#include "minilibc.h"
#include "system_config.h"
#include "defines.h"
#include "miner.h"
#include "io.h"

#include "api_test_data.c"	/* The test data array */

static struct lm32_api *api = (struct lm32_api *)API_BASE;

unsigned int api_set_cpm(unsigned int NR, unsigned int NF, unsigned int OD, unsigned int NB, unsigned int div)
{
	unsigned int div_loc = 0;
	unsigned int NR_sub;
	unsigned int NF_sub;
	unsigned int OD_sub;
	unsigned int NB_sub;

	NR_sub = NR - 1;
	NF_sub = NF - 1;
	OD_sub = OD - 1;
	NB_sub = NB - 1;
	
	if(div == 1  ) div_loc = 0;
	if(div == 2  ) div_loc = 1;
	if(div == 4  ) div_loc = 2;
	if(div == 8  ) div_loc = 3;
	if(div == 16 ) div_loc = 4;
	if(div == 32 ) div_loc = 5;
	if(div == 64 ) div_loc = 6;
	if(div == 128) div_loc = 7;
	
	return 0x7 | (div_loc << 7) | (1 << 10) |
		(NR_sub << 11) | (NF_sub << 15) | (OD_sub << 21) | (NB_sub << 25);
}

static inline unsigned int api_gen_test_work(unsigned int i, unsigned int chip_under_test_num, unsigned int * data)
{
	unsigned int tmp = 0;
	int j;

	for (j = 0; j < 18; j++) {
		data[j] = test_data[i%16][j];
	}

	tmp = data[0];
        data[0] = data[0] ^ (i & 0xfffffff0);//nonce
        data[2] = data[2] - chip_under_test_num;//role time

        data[18] = 0x0;//nonce2
        data[19] = i;//nonce2
        data[20] = 0x1;//cpm2
        data[21] = 0x1;//cpm1
        data[22] = 0x1;//cpm0

	return tmp + 0x18000;
}

static inline void api_set_num(unsigned int ch_num, unsigned int chip_num)
{
	unsigned int tmp;
	tmp = readl(&api->sck) & 0xff;
	tmp = (ch_num << 16) | (chip_num << 24) | tmp;
	writel(tmp, &api->sck);
}

static inline void api_set_sck(unsigned int spi_speed)
{
	unsigned int tmp;
	tmp = (readl(&api->sck) & 0xffff0000) | spi_speed;
	writel(tmp, &api->sck);
}

void api_initial(unsigned int ch_num, unsigned int chip_num, unsigned int spi_speed, unsigned int timeout)
{
	api_set_num(ch_num, chip_num);
	api_set_sck(spi_speed);
}

void api_set_timeout(unsigned int timeout)
{
	writel(timeout, &api->timeout);
}

void api_set_flush()
{
	writel(0x2, &api->state);
}

unsigned int api_get_tx_cnt()
{
	return (readl(&api->state) >> 2) && 0x3ff;
}

unsigned int api_get_rx_cnt()
{
	return (readl(&api->state) >> 20) & 0x1ff;
}

void api_set_tx_fifo(unsigned int * data)
{
	int i;
	for (i = 0; i < 23; i++)
		writel(data[i], &api->tx);
}

void api_get_rx_fifo(unsigned int * data)
{
	int i;
	for (i = 0; i < 4; i++)
		data[i] = readl(&api->rx);
}

static inline void api_wait_done(unsigned int ch_num, unsigned int chip_num)
{
	while (api_get_rx_cnt() != (ch_num * chip_num * 4))
		;
}

static inline unsigned int api_verify_nonce(unsigned int ch_num, unsigned int chip_num, unsigned int chip_under_test_num, unsigned int verify_on, unsigned int target_nonce)
{
	unsigned int i, j, need_verify;
	unsigned int rx_data[4];
	unsigned int pass_cal_num = 0;
	for (i = 0; i < ch_num; i++) {
		for (j = 0; j < chip_num; j++) {
			api_get_rx_fifo(rx_data);
			need_verify = ((chip_num - chip_under_test_num - 1) == j) ? 1 : 0;
			if(verify_on && (rx_data[2] == target_nonce) && need_verify)
				pass_cal_num++;
		}
	}
	return pass_cal_num;
}

unsigned int api_asic_test(unsigned int ch_num, unsigned int chip_num, unsigned int cal_core_num)
{
	unsigned int i, j, k;
	unsigned int tx_data[23];
	unsigned int target_nonce;
	unsigned int pass_cal_num = 0;
	unsigned int verify_on = 0;
	unsigned int spi_speed = 0x1;
	unsigned int timeout = 0x4318c63/2;

	api_initial(ch_num, chip_num, spi_speed, timeout);

	for (i = 0; i < chip_num; i++){
		for (j = 0; j < cal_core_num + 2; j++){
			api_gen_test_work(j, i, tx_data);
			if (!i && !j) {
				tx_data[20] = api_set_cpm(1, 16, 1, 16, 2);
				tx_data[21] = api_set_cpm(1, 16, 1, 16, 2);
				tx_data[22] = api_set_cpm(1, 16, 1, 16, 2);
			}
			for (k = 0; k < ch_num; k++)
				api_set_tx_fifo(tx_data);

			api_wait_done(ch_num, chip_num);
			target_nonce = api_gen_test_work((j-2)%16, 0, tx_data);
			verify_on = j >= 2 ? 1 : 0;
			pass_cal_num += api_verify_nonce(ch_num, chip_num, i, verify_on, target_nonce);
		}
	}
	return pass_cal_num;
}

int api_send_work(struct work *w)
{
	uint32_t tmp;
	int i, j, k;
	uint32_t data[18];

	/* ----------------------------------- */
	j = 0;

	/* Task data */
	for (i = 8; i >= 0; i -= 4) {
		memcpy((uint8_t *)(&tmp), w->data + 32 + i, 4);
		data[j++] = tmp;
	}

	memcpy((uint8_t *)(&tmp), w->a1, 4);
	data[j++] = tmp;
	memcpy((uint8_t *)(&tmp), w->a0, 4);
	data[j++] = tmp;
	memcpy((uint8_t *)(&tmp), w->e2, 4);
	data[j++] = tmp;
	memcpy((uint8_t *)(&tmp), w->e1, 4);
	data[j++] = tmp;
	memcpy((uint8_t *)(&tmp), w->e0, 4);
	data[j++] = tmp;

	for (i = 28; i >= 0; i -= 4) {
		memcpy((uint8_t *)(&tmp), w->data + i, 4);
		data[j++] = tmp;
	}

	memcpy((uint8_t *)(&tmp), w->a2, 4);
	data[j++] = tmp;
	/* ----------------------------------- */


	writel(0, &api->tx);
	for (k = j - 1; k >= 0; k--)
		writel(data[k], &api->tx);

	memcpy((uint8_t *)(&tmp), w->task_id, 4);
	writel(tmp, &api->tx);

	memcpy((uint8_t *)(&tmp), w->task_id + 4, 4);
	writel(tmp, &api->tx);

	/* The chip configure information */
	memcpy((uint8_t *)(&tmp), w->clock + 8, 4);
	writel(1, &api->tx);

	memcpy((uint8_t *)(&tmp), w->clock + 4, 4);
	writel(1, &api->tx);

	memcpy((uint8_t *)(&tmp), w->clock, 4);
	writel(1, &api->tx);

	debug32("TX:");
	hexdump(w->task_id, 8);
	hexdump(w->clock, 12);
	return 0;
}
