/*
 * @brief public functions
 *
 * @note
 * Author: Mikeqin Fengling.Qin@gmail.com
 *
 *
 * @par
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */
#include "board.h"
#include "libfunctions.h"

void delay(unsigned int ms)
{
	unsigned int i;
	unsigned int msticks = SystemCoreClock/16000; /* FIXME: 16000 is not accurate */

	while (ms && ms--) {
		for(i = 0; i < msticks; i++)
			__NOP();
	}
}
