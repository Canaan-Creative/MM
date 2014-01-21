/*
 * Author: Xiangfu Liu <xiangfu@openmobilefree.net>
 * Bitcoin:	1CanaaniJzgps8EV6Sfmpb7T8RutpaeyFn
 *
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */

#include "system_config.h"
#include "io.h"
#include "shifter.h"

static struct lm32_shifter *sft = (struct lm32_shifter *)SHIFTER_BASE;
static uint32_t g_voltage = 0x8f00; /* 0V */

static void shift_done()
{
	unsigned int tmp;
	tmp = readl(&sft->reg) & 0x8;

	while(tmp != 0x8)
		tmp = readl(&sft->reg) & 0x8;
}

uint32_t get_voltage()
{
	return g_voltage;
}

/* NOTICE: Always delay 10ms after set voltage */
extern void delay(unsigned int ms);

#define VOLTAGE_DELAY	50
void set_voltage(uint32_t value)
{
	int i;

	if (g_voltage == value)
		return;

	g_voltage = value;

	if (value == 0x8f00) {
		writel(0x7, &sft->reg);
		delay(VOLTAGE_DELAY);
		return;
	}

	/* Reset */
	writel(0, &sft->reg);

	/* The power chip datasheet is here:
	 *   http://www.onsemi.com/pub_link/Collateral/ADP3208D.PDF
	 * REV_BITS((VALUE < 1) & 1) << 16: is the value, the */

	/* Set shifter to xx */
	for (i = 0; i < 5; i++) {
		writel(value | 0x1, &sft->reg);
		shift_done();
	}

	/* Shift to reg */
	for (i = 0; i < 5; i++) {
		writel(0x2, &sft->reg);
		shift_done();
	}

	/* Output enable, low active  */
	writel(0x3, &sft->reg);
	delay(VOLTAGE_DELAY);
}
