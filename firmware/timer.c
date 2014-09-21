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
#include "intr.h"

static struct lm32_timer *tim = (struct lm32_timer *)TIMER_BASE;
static struct lm32_gpio *gpio = (struct lm32_gpio *)GPIO_BASE;
static struct lm32_clko *clko = (struct lm32_clko *)CLKO_BASE;

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

void timer_int_clean(unsigned char timer)
{
	unsigned int tmp;
	if (timer == 0) {
		tmp = readl(&tim->reg) & 0x00002;	/*read timer int mask*/
		tmp = tmp | 0x00000100;	/* clean */
	} else {
		tmp = readl(&tim->reg) & 0x20000;	/*read timer int mask*/
		tmp = tmp | 0x01000000;	/* clean */
	}
	writel(tmp, &tim->reg);
}

void timer0_isr(void)
{
	/* DO SOMETHING */

	timer_int_clean(0);
	irq_ack(IRQ_TIMER0);
}

void timer1_isr(void)
{
	/* DO SOMETHING */

	timer_int_clean(1);
	irq_ack(IRQ_TIMER0);
}

/* GPIO */
void gpio_frond_led(uint8_t)
{

}

void gpio_led(uint8_t led)
{
	uint32_t value;

	value = readl(&gpio->reg) & 0xffffff0f;
	value |= led << 4;

	writel(value, &gpio->reg);
}

extern void delay(unsigned int ms);
void gpio_reset_asic()
{
	uint32_t tmp;

	clko_init(1);

	tmp = readl(&gpio->reg);
	writel(tmp | 0xc, &gpio->reg);
	delay(100);

	writel(tmp & 0xfffffff3, &gpio->reg);
	delay(100);

	writel(tmp | 0xc, &gpio->reg);
	delay(100);
}

int read_power_good()
{
#if defined(AVALON3_A3233_MACHINE)
	return (readl(&gpio->reg) >> 7) & 0x1f;
#elif defined(AVALON3_A3233_CARD)
	return (readl(&gpio->reg) >> 10) & 0x3;
#elif defined(AVALON4_A3222)
	return (readl(&gpio->reg)) >> 16 & 0x3ff;
#endif
}

#if defined(AVALON3_A3233_MACHINE) || defined(AVALON3_A3233_CARD)
/* CLKO:
 * 1: enable
 * 0: disable
 * By default it is 0 */
int clko_init(uint32_t value)
{
	return (writel(value, &clko->reg));
}
#endif
