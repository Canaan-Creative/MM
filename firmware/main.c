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

static void set_led(uint32_t led) {
	volatile uint32_t *gpio_pio_data = (uint32_t *)GPIO_BASE;

	*gpio_pio_data = led;
}

static void delay(volatile uint32_t i) {
	while (i--)
		;
}

int main(void) {
	struct lm32_uart *uart0 = (struct lm32_uart *)UART0_BASE;
	uint32_t j = 1;

	while (1) {
		delay(16000000);

		j++;
		set_led(0x00345678 | (j << 24));

		uart0->rxtx = j & 0x000000ff;
	}

	return 0;
}

// vim: set ts=4 sw=4 fdm=marker :
