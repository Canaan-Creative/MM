/*
 * Author: Xiangfu Liu <xiangfu@openmobilefree.net>
 * Bitcoin:	1CanaaniJzgps8EV6Sfmpb7T8RutpaeyFn
 *
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */

#ifndef _API_H_
#define _API_H_

#include <stdint.h>

void api_initial(uint32_t ch_num, uint32_t chip_num, uint32_t spi_speed);

uint32_t api_get_tx_cnt(void);
uint32_t api_get_rx_cnt(void);

void api_get_rx_fifo(uint32_t *data);
void api_set_timeout(uint32_t timeout);

int api_send_work(struct work *w);

void set_asic_freq(uint32_t value[]);
void set_asic_freq_i(uint32_t cpm[]);
uint32_t get_asic_freq(void);

int api_asic_testcores(uint32_t cal_core_num, uint32_t ret);
void api_get_lw(uint32_t *buf);

void api_set_pll(uint32_t miner_id, uint32_t chip_id, uint32_t pll_data0, uint32_t pll_data1, uint32_t pll_data2);
uint32_t api_get_pll_fifo_count(void);
uint32_t api_get_pll_fifo_full(void);
uint32_t api_get_pll_fifo_empty(void);
void api_get_pll_fifo_reset(void);

#endif	/* _API_H_ */
