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
static uint32_t g_voltage_i[10];
static int32_t g_led = 0;

static void shift_done(struct lm32_shifter *s)
{
	unsigned int tmp;
	tmp = readl(&s->reg) & 0x8;

	while(tmp != 0x8)
		tmp = readl(&s->reg) & 0x8;
}

static void shift_update(struct lm32_shifter *s, uint32_t value[], int poweron)
{
	int i;

	if ((value[0] == ASIC_0V) && (value[1] == ASIC_0V) &&
		(value[2] == ASIC_0V) && (value[3] == ASIC_0V) &&
		(value[4] == ASIC_0V)) {
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
		writel(value[i] | 0x1, &s->reg);
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
#ifdef MM40
	uint32_t ret = 0;
	int poweron = 0;
	uint8_t i;

	if (g_voltage == value)
		return 0;

	if (g_voltage == ASIC_0V)
		poweron = 1;

	for (i = 0; i < 10; i++)
		g_voltage_i[i] = value;

	shift_update(sft0, g_voltage_i, poweron);
	shift_update(sft1, g_voltage_i + 5, poweron);

	if (g_voltage == ASIC_0V) {
		gpio_reset_asic();
		ret = 1;
	}

	g_voltage = value;

	return ret;
#else
	g_voltage = value;
	return 0;
#endif
}

/* Must call set_voltage first(record g_voltage), Call from AVA4_P_SET_VOLT, Only for MM-4.1 */
uint32_t set_voltage_i(uint32_t value[])
{
#ifdef MM41
	uint32_t ret;
	uint8_t i, diff = 0, ch1 = 0, ch2 = 0, reset = 1;
	int poweron = 0;

	for (i = 0; i < 10; i++) {
		if (g_voltage_i[i] != value[i]) {
			g_voltage_i[i] = value[i];
			diff = 1;
			if (i < 5)
				ch1 = 1;
			else
				ch2 = 1;
		}

		if (g_voltage_i[i] == ASIC_0V)
			poweron = 1;
		else
			reset = 0;
	}

	if (!diff)
		return 0;

	if (ch1)
		shift_update(sft0, g_voltage_i, poweron);

	if (ch2)
		shift_update(sft1, g_voltage_i + 5, poweron);

	if (reset) {
		gpio_reset_asic();
		ret = 1;
	}

	return ret;
#else
	return 0;
#endif
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
