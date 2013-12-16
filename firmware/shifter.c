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

static void shift_done()
{
	unsigned int tmp;
        tmp = readl(&sft->reg) & 0x8;

	while(tmp != 0x8)
        	tmp = readl(&sft->reg) & 0x8;
}


void shift()
{
	int i;

	writel(0, &sft->reg);		 /* Reset */

	for (i = 0; i < 5; i++) {
		writel(0x8a00 | 0x1, &sft->reg);
		shift_done(); /* set shifter 1v */
	}

	for (i = 0; i < 5; i++) {
		writel(0x2, &sft->reg);
		shift_done(); /* shift to reg */
	}

	writel(0x3, &sft->reg);	/* output enable, low active  */
}

void adjust_voltage(uint32_t value)
{

}
