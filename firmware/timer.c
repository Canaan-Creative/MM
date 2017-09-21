/*
 * Author: Xiangfu Liu <xiangfu@openmobilefree.net>
 *         Mikeqin <Fengling.Qin@gmail.com>
 * Bitcoin:	1CanaaniJzgps8EV6Sfmpb7T8RutpaeyFn
 *
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */

#include "sdk/intr.h"
#include "system_config.h"
#include "defines.h"
#include "io.h"
#include "timer.h"
#include "atom.h"

static struct lm32_timer *tim = (struct lm32_timer *)TIMER_BASE;

static volatile uint32_t timerval[TIMER_MAX];
static timerproc timerfun[TIMER_MAX];

static void timer_mask_set(void)
{
	unsigned int tmp;

	tmp = readl(&tim->reg) & 0x20002;	/*read timer int mask*/
	tmp |= 0x102;
	writel(tmp, &tim->reg);
}

static void timer_mask_clean(void)
{
	unsigned int tmp;

	tmp = readl(&tim->reg) & 0x20002;	/*read timer int mask*/
	tmp &=  0xfffffffd;
	tmp |= 0x5;
	writel(tmp, &tim->reg);
}

void timer_init(void)
{
	enum timer_id	id;
	uint32_t tmp;

	for (id = TIMER_IDLE; id < TIMER_MAX; id++) {
		timerval[id] = 0;
		timerfun[id] = NULL;
	}

	/* timer 0 init */
	timer_mask_clean();

	irq_ack(IRQ_TIMER0);
	tmp = irq_getmask();
	tmp |= IRQ_TIMER0;
	irq_setmask(tmp);
}

void timer_set(enum timer_id id, uint32_t seconds, timerproc proc)
{
	timerval[id] = seconds;
	timerfun[id] = proc;
}

bool timer_istimeout(enum timer_id id)
{
	if (!timerval[id])
		return true;

	return false;
}

void timer0_isr(void)
{
	enum timer_id	id;

	timer_mask_set();
	for (id = TIMER_IDLE; id < TIMER_MAX; id++) {
		if (timerval[id] == MAX_TIMER_VAL)
			continue;

		if (timerval[id])
			timerval[id]--;

		if (!timerval[id]) {
			if (timerfun[id])
				timerfun[id]();
		}
	}

	timer_mask_clean();
	irq_ack(IRQ_TIMER0);

	atomIntEnter();
	atomTimerTick();
	atomIntExit(TRUE);
}

static void ticker_mask_set(void)
{
	unsigned int tmp;

	tmp = readl(&tim->reg) & 0x20002;	/*read timer int mask*/
	tmp |= (0x102 << 16);
	writel(tmp, &tim->reg);
}

static void ticker_mask_clean(void)
{
	unsigned int tmp;

	tmp = readl(&tim->reg) & 0x20002;	/*read timer int mask*/
	tmp &=  0xfffdffff;
	tmp |= (0x5 << 16);
	writel(tmp, &tim->reg);
}

void ticker_init(void)
{
	uint32_t tmp;

	ticker_mask_clean();

	irq_ack(IRQ_TIMER1);
	tmp = irq_getmask();
	tmp |= IRQ_TIMER1;
	irq_setmask(tmp);
}

void timer1_isr(void)
{
	ticker_mask_set();
	ticker_mask_clean();
	irq_ack(IRQ_TIMER1);
}
