/*
 * Author: Xiangfu Liu <xiangfu@openmobilefree.net>
 * Bitcoin:	1CanaaniJzgps8EV6Sfmpb7T8RutpaeyFn
 *
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */

#ifndef _API_H_
#define _API_H_

unsigned int api_get_tx_cnt();
unsigned int api_get_rx_cnt();

void api_get_rx_fifo(unsigned int *data);

int api_send_work(struct work *w);

unsigned int api_asic_test(unsigned int ch_num, unsigned int chip_num, unsigned int cal_core_num);

unsigned int api_set_cpm(unsigned int NR, unsigned int NF, unsigned int OD, unsigned int NB, unsigned int div);
void api_set_timeout(unsigned int timeout);


#endif	/* _API_H_ */
