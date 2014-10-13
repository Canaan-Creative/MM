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
#include "timer.h"

/* NOTICE: Always delay 100ms after set voltage */
extern void delay(unsigned int ms);
#define VOLTAGE_DELAY	100

static struct lm32_shifter *sft0 = (struct lm32_shifter *)SHIFTER_BASE0;
//static struct lm32_shifter *sft1 = (struct lm32_shifter *)SHIFTER_BASE1;
static struct lm32_shifter *sft2 = (struct lm32_shifter *)SHIFTER_BASE2;

static uint32_t g_voltage = ASIC_0V;

static void shift_done()
{
	unsigned int tmp;
	tmp = readl(&sft0->reg) & 0x8;

	while(tmp != 0x8)
		tmp = readl(&sft0->reg) & 0x8;
}

int set_voltage(uint32_t value)
{
	int i;

	if (g_voltage == value)
		return 0;

	g_voltage = value;

	if (value == ASIC_0V) {
		writel(0x7, &sft0->reg);
		delay(VOLTAGE_DELAY);
		return 0;
	}

	/* Reset */
	writel(0, &sft0->reg);

	/* The power chip datasheet is here:
	 *   http://www.onsemi.com/pub_link/Collateral/ADP3208D.PDF
	 * REV_BITS((VALUE < 1) & 1) << 16: is the value, the */

	/* Set shifter to xx */
	for (i = 0; i < 5; i++) {
		writel(value | 0x1, &sft0->reg);
		shift_done();
	}

	/* Shift to reg */
	for (i = 0; i < 5; i++) {
		writel(0x2, &sft0->reg);
		shift_done();
	}

	/* Output enable, low active  */
	writel(0x3, &sft0->reg);
	delay(VOLTAGE_DELAY);

	gpio_reset_asic();

	return 1;
}

uint32_t get_voltage()
{
	return g_voltage;
}

void front_led(uint8_t data)
{
	static uint32_t value = 0;

	if (value) {
		while (!(readl(&sft2->reg) & 0x4))
			;

		writel(0x4, &sft2->reg);
	} else
		value = 1;

	writel((data << 8) | 0x1, &sft2->reg);
}
