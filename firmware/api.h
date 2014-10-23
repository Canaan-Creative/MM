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

uint32_t api_get_tx_cnt();
uint32_t api_get_rx_cnt();

void api_set_flush();
void api_get_rx_fifo(uint32_t *data);
void api_set_timeout(uint32_t timeout);

int api_send_work(struct work *w);

void set_asic_freq(uint32_t value[]);
uint32_t get_asic_freq();

uint32_t api_asic_test(uint32_t ch_num, uint32_t chip_num,
		       uint32_t cal_core_num, uint32_t add_step,
		       uint32_t *pass_zone_num, uint32_t freq[], uint32_t result[MINER_COUNT][ASIC_COUNT]);
#endif	/* _API_H_ */
