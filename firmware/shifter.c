/*
 * Author: Xiangfu Liu <xiangfu@openmobilefree.net>
 * Bitcoin:	1CanaaniJzgps8EV6Sfmpb7T8RutpaeyFn
 *
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */

#include "system_config.h"
#include "defines.h"
#include "io.h"
#include "shifter.h"
#include "timer.h"

/* NOTICE: Always delay 100ms after set voltage */
#define VOLTAGE_DELAY	100

static struct lm32_shifter *sft0 = (struct lm32_shifter *)SHIFTER_BASE0;
static struct lm32_shifter *sft1 = (struct lm32_shifter *)SHIFTER_BASE1;
static struct lm32_shifter *sft2 = (struct lm32_shifter *)SHIFTER_BASE2;

static uint32_t g_voltage = ASIC_0V;
static int32_t g_led = 0;

static void shift_done(struct lm32_shifter *s)
{
	unsigned int tmp;
	tmp = readl(&s->reg) & 0x8;

	while(tmp != 0x8)
		tmp = readl(&s->reg) & 0x8;
}

static void shift_update(struct lm32_shifter *s, uint32_t value, int poweron)
{
	int i;

	if (value == ASIC_0V) {
		writel(0x7, &s->reg);
		return;
	}

	/* Reset */
	writel(0, &s->reg);

	/* The power chip datasheet is here:
	 *   http://www.onsemi.com/pub_link/Collateral/ADP3208D.PDF
	 * REV_BITS((VALUE < 1) & 1) << 16: is the value, the */

	/* Set shifter to xx */
	for (i = 0; i < 5; i++) {
		writel(value | 0x1, &s->reg);
		shift_done(s);
	}

	/* Shift to reg */
	for (i = 0; i < 5; i++) {
		writel(0x2, &s->reg);
		shift_done(s);
	}

	/* Output enable, low active  */
	writel(0x3, &s->reg);
	if (poweron)
		delay(VOLTAGE_DELAY);
}

uint32_t set_voltage(uint32_t value)
{
	uint32_t ret = 0, poweron = 0;

	if (g_voltage == value)
		return ret;

	if (g_voltage == ASIC_0V)
		poweron = 1;

	shift_update(sft0, value, poweron);
	shift_update(sft1, value, poweron);

	if (g_voltage == ASIC_0V) {
		gpio_reset_asic();
		ret = 1;
	}

	g_voltage = value;

	return ret;
}

uint32_t get_voltage(void)
{
	return g_voltage;
}

uint32_t get_front_led(void)
{
	return g_led;
}

void set_front_led(uint32_t value)
{
	if (g_led == value)
		return;

	g_led = value;
	writel(value, &sft2->reg);
}
