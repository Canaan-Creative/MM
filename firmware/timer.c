/*
 * Author: Xiangfu Liu <xiangfu@openmobilefree.net>
 * Bitcoin:	1CanaaniJzgps8EV6Sfmpb7T8RutpaeyFn
 *
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */

#include "system_config.h"
#include "io.h"
#include "timer.h"

static struct lm32_timer *tim = (struct lm32_timer *)TIMER_BASE;

void timer_mask_set(unsigned char timer)
{
	unsigned int tmp;
	tmp = readl(&tim->reg) & 0x20002;	/*read timer int mask*/
	tmp = timer ? (tmp | 0x20000) : (tmp | 0x2);
	writel(tmp, &tim->reg);
}

void timer_mask_clean(unsigned char timer)
{
	unsigned int tmp;
	tmp = readl(&tim->reg) & 0x20002;	/*read timer int mask*/
	tmp = timer ? (tmp & 0xfffdffff) : (tmp & 0xfffffffd);
	writel(tmp, &tim->reg);
}

/*
 * Timer Set
 * (1)timer = 0 or 1;
 * (2)load: the seconds for timer 0 or 1
 */
void timer_set(unsigned char timer, unsigned char load)
{
	unsigned int tmp;
	tmp = readl(&tim->reg) & 0x20002;	/*read timer int mask*/
	if (!timer)
		writel((load << 2) | 1 | tmp, &tim->reg);
	else
		writel((((load << 2) | 1) << 16) | tmp, &tim->reg);
}

/*
 * Timer read
 * (1)timer = 0 or 1;
 */
uint32_t timer_read(unsigned char timer)
{
	unsigned int tmp;
	if (timer == 0)
		tmp = (readl(&tim->reg) >> 2) & 0x3f;
	else
		tmp = (readl(&tim->reg) >> 18) & 0x3f;
	return tmp;
}

void timer_int_clean()
{
	unsigned int tmp;
	tmp = readl(&tim->reg) & 0x20002;	/*read timer int mask*/
	tmp = tmp | 0x01000100;	/* clean */
	writel(tmp, &tim->reg);
}
