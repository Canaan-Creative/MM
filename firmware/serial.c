/*
 * Author: Xiangfu Liu <xiangfu@openmobilefree.net>
 * Bitcoin:	1CanaaniJzgps8EV6Sfmpb7T8RutpaeyFn
 *
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */

#include "sdk.h"
#include "minilibc.h"

#include "system_config.h"
#include "io.h"

static struct lm32_uart *uart = (struct lm32_uart *)UART0_BASE;

static int serial_tstc(void)
{
	if (readb(&uart->lsr) & LM32_UART_LSR_DR)
		return 1;

	return 0;
}

void uart_init(void)
{
	uint8_t value;
	/* Disable UART interrupts */
	writeb(0, &uart->ier);

	/* Line control 8 bit, 1 stop, no parity */
	writeb(LM32_UART_LCR_8BIT, &uart->lcr);

	/* Modem control, DTR = 1, RTS = 1 */
	writeb(LM32_UART_MCR_DTR | LM32_UART_MCR_RTS, &uart->mcr);

	/* Set baud rate */
	value = (CPU_FREQUENCY / UART_BAUD_RATE) & 0xff;
	writeb(value, &uart->divl);
	value = (CPU_FREQUENCY / UART_BAUD_RATE) >> 8;
	writeb(value, &uart->divh);
}

unsigned char serial_getc()
{
	while (!serial_tstc())
		;

	return readb(&uart->rxtx);
}


void serial_putc(const unsigned char c)
{
	if (c == '\n')
		serial_putc('\r');

	/* Wait for fifo to shift out some bytes */
	while (!((readb(&uart->lsr) & (LM32_UART_LSR_THRR | LM32_UART_LSR_TEMT)) == 0x60))
		;

	writeb(c, &uart->rxtx);
}

void serial_puts(const char *s)
{
	while (*s)
		serial_putc(*s++);
}
