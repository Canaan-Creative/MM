/*
 * Author: Xiangfu Liu <xiangfu@openmobilefree.net>
 * Bitcoin:	1CanaaniJzgps8EV6Sfmpb7T8RutpaeyFn
 *
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */

#include <stdint.h>

#include "intr.h"
#include "system_config.h"
#include "timer.h"
#include "uart.h"

#define WEAK __attribute__ ((weak))
#define ALIAS(f) __attribute__ ((weak, alias (#f)))

WEAK void default_isr(void);

void timer0_isr(void) ALIAS(default_isr);
void timer1_isr(void) ALIAS(default_isr);
void uart_isr(void) ALIAS(default_isr);
void uart1_isr(void) ALIAS(default_isr);

void default_isr(void)
{
	while(1)
		;
}

void isr(void)
{
	unsigned int irqs;

	irqs = irq_pending() & irq_getmask();

	if (irqs & IRQ_TIMER0)
		timer0_isr();

	if (irqs & IRQ_TIMER1)
		timer1_isr();

	if (irqs & IRQ_UART)
		uart_isr();

	if (irqs & IRQ_UART1)
		uart1_isr();
}
