/*
 * Author: Xiangfu Liu <xiangfu@openmobilefree.net>
 * Bitcoin:	1CanaaniJzgps8EV6Sfmpb7T8RutpaeyFn
 *
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */

#ifndef _SERIAL_H
#define _SERIAL_H

unsigned char serial_getc(void);
void serial_putc(const unsigned char c);
void serial_puts(const char *s);

void uart_init(void);
void uart_isr(void);
int uart_read_nonblock(void);
char uart_read(void);
void uart_force_sync(int f);
void uart_write(char c);

#endif	/* _SERIAL_H */
