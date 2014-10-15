/*
 * Author: Xiangfu Liu <xiangfu@openmobilefree.net>
 * Bitcoin:	1CanaaniJzgps8EV6Sfmpb7T8RutpaeyFn
 *
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */

#ifndef _API_H_
#define _API_H_

void api_initial(unsigned int ch_num, unsigned int chip_num, unsigned int spi_speed);

unsigned int api_get_tx_cnt();
unsigned int api_get_rx_cnt();

void api_get_rx_fifo(unsigned int *data);
void api_set_timeout(unsigned int timeout);

int api_send_work(struct work *w);

void set_asic_freq(uint32_t value);
uint32_t get_asic_freq();

unsigned int api_asic_test(unsigned int ch_num, unsigned int chip_num, unsigned int cal_core_num, unsigned int add_step, unsigned int *pass_zone_num);
#endif	/* _API_H_ */
