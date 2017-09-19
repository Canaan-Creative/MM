/*
 * Author: Mikeqin <Fengling.Qin@gmail.com>
 *         xuzhenxing <xuzhenxing@canaan-creative.com>
 *
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */

#include <stdbool.h>
#include <stdint.h>

#include "minilibc.h"
#include "system_config.h"
#include "defines.h"
#include "io.h"
#include "intr.h"
#include "uart.h"
#include "timer.h"
#include "gpio.h"
#include "atom.h"
#include "atomtests.h"

/* Thread Stack Size define */
#define IDLE_STACK_SIZE_BYTES	256
#define FIRST_STACK_SIZE_BYTES	1024
#define SECOND_STACK_SIZE_BYTES	1024

/* Local data */
/* Application thread's TCBs */
ATOM_TCB first_tcb;
ATOM_TCB second_tcb;

/* Idle thread's stack area */
uint8_t idle_thread_stack[IDLE_STACK_SIZE_BYTES];
/* First thread's stack area */
uint8_t first_thread_stack[FIRST_STACK_SIZE_BYTES];
/* Second thread's stack area */
uint8_t second_thread_stack[SECOND_STACK_SIZE_BYTES];

/* Forward declarations */
void first_thread_func(uint32_t data);
void second_thread_func(uint32_t data);

void first_thread_func(uint32_t data)
{
	debug32("test start = %d\n", test_start());

	while (1) {
		debug32("thread-1 start.\n");
		gpio_led(0);
		atomTimerDelay(3);
		gpio_led(1);
		atomTimerDelay(3);
		debug32("thread-1 end.\n");
	}
}

void second_thread_func(uint32_t data)
{
	while (1) {
		debug32("thread-2 start.\n");
		gpio_led_rgb(GPIO_LED_BLACK);
		atomTimerDelay(1);
		gpio_led_rgb(GPIO_LED_RED);
		atomTimerDelay(1);
		debug32("thread-2 end.\n");
	}
}

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
	int8_t status;

	debug32("LM32 start\n");

	/* Initialise the OS before creating our threads */
	status = atomOSInit(&idle_thread_stack[0], IDLE_STACK_SIZE_BYTES, TRUE);
	if (status == ATOM_OK) {
		/* Enable the system */
		irq_setmask(0);
		irq_enable(1);
		timer_init();
#ifdef DEBUG
		uart2_init();
#endif
		/* Create an application thread */
		status = atomThreadCreate(&first_tcb,
				10, first_thread_func, 3,
				&first_thread_stack[0],
				FIRST_STACK_SIZE_BYTES,
				TRUE);
		status |= atomThreadCreate(&second_tcb,
				254, second_thread_func, 6,
				&second_thread_stack[0],
				SECOND_STACK_SIZE_BYTES,
				TRUE);
		if (status == ATOM_OK) {
			/**
			 * Frist application thread successfully created. It is
			 * now possible to start the OS. Execution will not return
			 * our atomOSStart(), which will restore the context of
			 * out application thread and start executing it.
			 *
			 * Note that interrrupts are still disabled at this point.
			 * They will be enabled as we restore and execute our first
			 * thread in archFirstThreadRestore().
			 */
			atomOSStart();
		}
	}

	return 0;
}
