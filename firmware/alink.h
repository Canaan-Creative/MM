/*
 * Author: Xiangfu Liu <xiangfu@openmobilefree.net>
 * Bitcoin:	1CanaaniJzgps8EV6Sfmpb7T8RutpaeyFn
 *
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */

#ifndef _ALINK_H_
#define _ALINK_H_

void alink_init();
void alink_buf_status();
int alink_txbuf_full();
int alink_send_work(struct work *w);
int alink_rxbuf_empty();
void alink_read_result(struct result *r);

void send_test_work();

#endif	/* _ALINK_H_ */
