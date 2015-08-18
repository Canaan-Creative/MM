/*
 * @brief
 *
 * @note
 * Author: Mikeqin Fengling.Qin@gmail.com
 *
 * @par
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */

#include "board.h"
#include "avalon_timer.h"

/* 1ms */
#define TICKRATE	1000

struct timer_node {
	bool	enable;
	unsigned int	curval;
	unsigned int	initval;
	TMRPROC	tmrfun;
	bool	istimeout;
};

struct timer_node tmrlist[TIMER_MAX];

void TIMER32_0_IRQHandler(void)
{
	enum timer_id	id;

	if (Chip_TIMER_MatchPending(LPC_TIMER32_0, 1)) {
		Chip_TIMER_ClearMatch(LPC_TIMER32_0, 1);
		for (id = TIMER_ID1; id < TIMER_MAX; id++) {
			if (tmrlist[id].enable && tmrlist[id].curval) {
				tmrlist[id].curval--;
				if (!tmrlist[id].curval) {
					if (tmrlist[id].tmrfun)
						tmrlist[id].tmrfun();
					else
						tmrlist[id].istimeout = true;
					tmrlist[id].curval = tmrlist[id].initval;
				}
			}
		}
	}
}

void timer_init(void)
{
	enum timer_id	id;
	uint32_t	timer_freq;

	for (id = TIMER_ID1; id < TIMER_MAX; id++) {
		tmrlist[id].enable = false;
		tmrlist[id].curval = 0;
		tmrlist[id].initval = 0;
		tmrlist[id].tmrfun = NULL;
		tmrlist[id].istimeout = false;
	}

	/* Enable timer 1 clock */
	Chip_TIMER_Init(LPC_TIMER32_0);

	/* Timer rate is system clock rate */
	timer_freq = Chip_Clock_GetSystemClockRate();

	/* Timer setup for match and interrupt at TICKRATE_HZ */
	Chip_TIMER_Reset(LPC_TIMER32_0);
	Chip_TIMER_MatchEnableInt(LPC_TIMER32_0, 1);
	Chip_TIMER_SetMatch(LPC_TIMER32_0, 1, (timer_freq / TICKRATE));
	Chip_TIMER_ResetOnMatchEnable(LPC_TIMER32_0, 1);

	/* Enable timer interrupt */
	NVIC_ClearPendingIRQ(TIMER_32_0_IRQn);
	NVIC_EnableIRQ(TIMER_32_0_IRQn);
	Chip_TIMER_Enable(LPC_TIMER32_0);
}

/* tmrcb must be run little time or else use timer_getready */
void timer_set(enum timer_id id, unsigned int interval, TMRPROC tmrcb)
{
	tmrlist[id].enable = true;
	tmrlist[id].curval = interval;
	tmrlist[id].initval = interval;
	tmrlist[id].tmrfun = tmrcb;
	tmrlist[id].istimeout = false;
}

void timer_kill(enum timer_id id)
{
	if (tmrlist[id].enable)
		tmrlist[id].enable = false;
}

enum timer_id timer_getready(void)
{
	static enum timer_id id = TIMER_ID1;

	if (id == TIMER_MAX)
		id = TIMER_ID1;

	for (; id < TIMER_MAX; id++) {
		if (tmrlist[id].enable && tmrlist[id].istimeout) {
			tmrlist[id].istimeout = false;
			break;
		}
	}
	return id;
}

bool timer_istimeout(enum timer_id id)
{
	if (tmrlist[id].enable && tmrlist[id].istimeout)
		return true;

	return false;
}

unsigned int timer_elapsed(enum timer_id id)
{
	if (tmrlist[id].enable)
		return tmrlist[id].initval - tmrlist[id].curval;

	return 0;
}

