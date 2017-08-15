/*
 * Author: Mikeqin <Fengling.Qin@gmail.com>
 *
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "minilibc.h"
#include "system_config.h"
#include "defines.h"
#include "io.h"
#include "intr.h"
#include "uart.h"
#include "timer.h"
#include "gpio.h"

void delay(unsigned int ms)
{
	unsigned int i;

	while (ms && ms--) {
		for (i = 0; i < CPU_FREQUENCY / 1000 / 5; i++)
			__asm__ __volatile__("nop");
	}
}

int main(int argv, char **argc)
{
	static uint8_t led_on = 0;

	irq_setmask(0);
	irq_enable(1);

	timer_init();
#ifdef DEBUG
	uart2_init();
#endif
	debug32("MM-%s\n", MM_VERSION);
	gpio_led(0);
	gpio_led_rgb(GPIO_LED_BLACK);
	timer_set(TIMER_IDLE, IDLE_TIME, NULL);

	while (1) {
		if (timer_istimeout(TIMER_IDLE)) {
			debug32("D: IDLE_TIME -> 0\n");
			gpio_led(led_on);
			led_on = !led_on;
			timer_set(TIMER_IDLE, IDLE_TIME, NULL);
		}
	}

	return 0;
}
