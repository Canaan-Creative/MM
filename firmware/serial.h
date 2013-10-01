/*
 * Author: Xiangfu Liu <xiangfu@openmobilefree.net>
 * Bitcoin:	1CanaaniJzgps8EV6Sfmpb7T8RutpaeyFn
 *
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */

#ifndef _SERIAL_H
#define _SERIAL_H

void uart_init(void);
unsigned char serial_getc();
void serial_putc(const unsigned char c);

#endif	/* _SERIAL_H */
