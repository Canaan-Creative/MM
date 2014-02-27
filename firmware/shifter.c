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

#define VOLTAGE_DELAY	200
static void shifte_value(uint32_t *value, int count)
{
	int i;

	/* Set shifter to xx [FPGA 0 1 2 3 4 FAN] */
	for (i = 0; i < count; i++) {
		writel(value[i] | 0x1, &sft->reg);
		shift_done();
	}

	/* Shift to reg */
	for (i = 0; i < count; i++) {
		writel(0x2, &sft->reg);
		shift_done();
	}

	/* Output enable, low active  */
	writel(0x3, &sft->reg);
}


/* The power chip datasheet is here:
 *   http://www.onsemi.com/pub_link/Collateral/ADP3208D.PDF
 * REV_BITS((VALUE < 1) & 1) << 16: is the value, the */

void set_voltage(uint32_t value)
{
	int i;
	uint32_t value_array[5];

	if (g_voltage == value)
		return;

	g_voltage = value;

	/* Reset */
	writel(0, &sft->reg);

	if (value == 0x8f00) {	/* Poweroff */
		writel(0x7, &sft->reg);
		delay(VOLTAGE_DELAY);
		return;
	}

	for (i = 0; i < 5; i++)
		value_array[i] = 0x8f00;

	/* Poweron one by one */
	for (i = 0; i < 5; i++) {
		value_array[i] = value;
		shifte_value(value_array, i);
		delay(VOLTAGE_DELAY);
	}
}
