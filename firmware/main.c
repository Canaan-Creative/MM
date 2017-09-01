/*
 * Author: Mikeqin <Fengling.Qin@gmail.com>
 * Author: xuzhenxing <xuzhenxing@canaan-creative.com>
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
#include "atomport.h"
#include "atommutex.h"
#include "atomsem.h"
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

/* Test OS objects */
ATOM_MUTEX mutex;
ATOM_SEM sem;

void first_thread_func(uint32_t data)
{
	static uint8_t led_on = 0;
	int test_status = 0;

	if (data == 3)
		debug32("input param is right.\n");
	else
		debug32("input param is wrong.\n");

	/* tests start */
	test_status = test_start();
	if (!test_status)
		debug32("test status is 0.\n");
	else
		debug32("test status is %d.\n", test_status);

	while (1) {
		atomTimerDelay(3);
		delay(1000);
		debug32("first thread func.\n");
		gpio_led(led_on);
		delay(1000);
		gpio_led(!led_on);
		delay(1000);
	}
}

void second_thread_func(uint32_t data)
{
	static uint8_t led_on = 0;

	if (data == 6)
		debug32("second input param is right.\n");
	else
		debug32("second input param is wrong.\n");

	while (1) {
		debug32("second thread func.\n");
		gpio_led(led_on);
		delay(100);
		gpio_led(!led_on);
		delay(100);
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
	static uint8_t led_on = 0;
	int8_t status;

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
		gpio_led(led_on);
		gpio_led_rgb(GPIO_LED_BLACK);

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
			debug32("create thread success.\n");

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
		else
			debug32("create thread failed!\n");
	}

	return 0;
}
