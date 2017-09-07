/*
 * Author: Mikeqin <Fengling.Qin@gmail.com>
 *         xuzhenxing <xuzhenxing@canaan-creative.com>
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
#include "atom.h"
#include "atomtests.h"

#define IDLE_STACK_SIZE_BYTES	1024
#define FIRST_STACK_SIZE_BYTES	1024
#define SECOND_STACK_SIZE_BYTES	1024

static ATOM_TCB first_tcb;
static ATOM_TCB second_tcb;

static uint8_t idle_thread_stack[IDLE_STACK_SIZE_BYTES];
static uint8_t first_thread_stack[FIRST_STACK_SIZE_BYTES];
static uint8_t second_thread_stack[SECOND_STACK_SIZE_BYTES];

static void first_thread_func(uint32_t data);
static void second_thread_func(uint32_t data);

static void first_thread_func(uint32_t data)
{
	debug32("%s : ", TESTCASE);
	debug32("ret = %d\n", test_start());

	while (1) {
		gpio_led(0);
		atomTimerDelay(2000);
		gpio_led(1);
		atomTimerDelay(2000);
	}
}

static void second_thread_func(uint32_t data)
{
	while (1) {
		gpio_led_rgb(GPIO_LED_BLACK);
		atomTimerDelay(1000);
		gpio_led_rgb(GPIO_LED_RED);
		atomTimerDelay(1000);
	}
}

int main(int argv, char **argc)
{
	int8_t status;

	/* Initialise the OS before creating our threads */
	status = atomOSInit(&idle_thread_stack[0], IDLE_STACK_SIZE_BYTES, TRUE);
	if (status == ATOM_OK) {
		irq_setmask(0);
		irq_enable(1);

		ticker_init();
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
