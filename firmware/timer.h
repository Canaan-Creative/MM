/*
 * Author: Mikeqin <Fengling.Qin@gmail.com>
 *
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */

#ifndef __TIMER_H__
#define __TIMER_H__

#include <stdbool.h>
#include "atom.h"

#define MAX_TIMER_VAL	(uint32_t)-1

enum timer_id {
    TIMER_IDLE,
    TIMER_MAX
};

typedef int (*timerproc)(void);

void timer_init(void);
void timer_set(enum timer_id id, uint32_t seconds, timerproc proc);
bool timer_istimeout(enum timer_id id);
void timer0_isr(void);

#endif	/* __TIMER_H__ */
