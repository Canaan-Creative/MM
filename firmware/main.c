//
// Author: Minux
// Author: Xiangfu Liu <xiangfu@openmobilefree.net>
// Bitcoin:	1CanaaniJzgps8EV6Sfmpb7T8RutpaeyFn
//
// This is free and unencumbered software released into the public domain.
// For details see the UNLICENSE file at the root of the source tree.
//

#include "sdk.h"
#include "minilibc.h"

#include "system_config.h"
#include "io.h"

struct lm32_uart *uart = (struct lm32_uart *)UART0_BASE;

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

static int serial_tstc(void)
{
	if (readb(&uart->lsr) & LM32_UART_LSR_DR)
		return 1;

	return 0;
}

static unsigned char serial_getc()
{
	while (!serial_tstc())
		;

	return readb(&uart->rxtx);
}


static void serial_putc(const unsigned char c)
{
	if (c == '\n')
		serial_putc('\r');

	/* Wait for fifo to shift out some bytes */
	while (!((readb(&uart->lsr) & (LM32_UART_LSR_TDRR | LM32_UART_LSR_TEMT)) == 0x60))
		;

	writeb(c, &uart->rxtx);
}

static void set_led(uint32_t led)
{
	volatile uint32_t *gpio_pio_data = (uint32_t *)GPIO_BASE;

	*gpio_pio_data = led;
}

static void delay(volatile uint32_t i)
{
	while (i--)
		;
}

const char *result = "\n\n{\"params\": ["
	"\"userid\"," // miner id
	"\"263884\", " // job id
	"\"62000000\"," // extra nonce
	"\"5214f239\"," // ntime
	"\"26a8c33c\"" // nonce
	"],"
	"\"id\": 297,"
	"\"method\": \"mining.submit\"}\n\n";


int main(void) {
	uint8_t tmp;
	uint32_t j = 0;

	uart_init();

	while (result[j]) {
		serial_putc(result[j++]);
	}
	while (1) {
		delay(4000000);

		j++;
		set_led(0x00345678 | (j << 24));

		tmp = serial_getc();
		serial_putc(tmp);
	}

	return 0;
}

// vim: set ts=4 sw=4 fdm=marker :
