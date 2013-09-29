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

static void uart_init(struct lm32_uart *uart, int baud)
{
	/* Disable UART interrupts */
	writeb(0, &uart->ier);

	/* Line control 8 bit, 1 stop, no parity */
	writeb(LM32_UART_LCR_8BIT, &uart->lcr);

	/* Modem control, DTR = 1, RTS = 1 */
	writeb(LM32_UART_MCR_DTR | LM32_UART_MCR_RTS, &uart->mcr);

	/* Set baud rate */
	writew(bswap_16(CPU_FREQUENCY / baud), &uart->div);
}

int main(void) {
	struct lm32_uart *uart0 = (struct lm32_uart *)UART0_BASE;
	uint32_t j = 1;

	uart_init(uart0, UART_BAUD_RATE);

	while (1) {
		delay(16000000);

		j++;
		set_led(0x00345678 | (j << 24));

		if (j % 2)
			uart0->rxtx = 0xaa;
		else
			uart0->rxtx = 0x55;
	}

	return 0;
}

// vim: set ts=4 sw=4 fdm=marker :
