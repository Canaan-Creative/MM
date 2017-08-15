/*
 * Author: Xiangfu Liu <xiangfu@openmobilefree.net>
 *         Mikeqin <Fengling.Qin@gmail.com>
 * Bitcoin:	1CanaaniJzgps8EV6Sfmpb7T8RutpaeyFn
 *
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */

#ifndef _UART_H
#define _UART_H

void uart_init(void);
void uart_isr(void);
int uart_read_nonblock(void);
char uart_read(void);
void uart_write(char c);
void uart_puts(const char *s);
void uart_nwrite(const char *s, unsigned int l);

void uart1_init(void);
void uart1_isr(void);
int uart1_read_nonblock(void);
char uart1_read(void);
void uart1_write(char c);
void uart1_puts(const char *s);
void uart1_nwrite(const char *s, unsigned int l);

#ifdef DEBUG
void uart2_init(void);
void uart2_write(char c);
void uart2_puts(const char *s);
#endif
#endif	/* _UART_H */
