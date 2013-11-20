/*
 * Author: Xiangfu Liu <xiangfu@openmobilefree.net>
 * Bitcoin:	1CanaaniJzgps8EV6Sfmpb7T8RutpaeyFn
 *
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */

#include <stdint.h>
#include "minilibc.h"
#include "system_config.h"
#include "defines.h"
#include "io.h"


static struct lm32_twipwm *tp = (struct lm32_twipwm *)TWIPWM_BASE;

void twi_start(void)
{
	writel(LM32_TWIPWM_CR_ENABLE | LM32_TWIPWM_CR_TSTART, &tp->cr);
	while((readl(&tp->cr) & LM32_TWIPWM_CR_TDONE) != LM32_TWIPWM_CR_TDONE)
		;
}

void twi_write(uint8_t value)
{
	writel(value, &tp->wd);
	writel(LM32_TWIPWM_CR_ENABLE | LM32_TWIPWM_CR_TSTART | LM32_TWIPWM_CR_CMD_WD
	       , &tp->cr);
	while((readl(&tp->cr) & LM32_TWIPWM_CR_TDONE) != LM32_TWIPWM_CR_TDONE)
		;
}

uint8_t twi_read(void)
{
	writel(LM32_TWIPWM_CR_ENABLE | LM32_TWIPWM_CR_TSTART | LM32_TWIPWM_CR_CMD_RDACK
	       , &tp->cr);
	while((readl(&tp->cr) & LM32_TWIPWM_CR_TDONE) != LM32_TWIPWM_CR_TDONE)
		;

	return readb(&tp->rd);
}

void twi_stop(void)
{
	writel(LM32_TWIPWM_CR_ENABLE | LM32_TWIPWM_CR_TSTART | LM32_TWIPWM_CR_CMD_STOP
	       , &tp->cr);
	while((readl(&tp->cr) & LM32_TWIPWM_CR_TDONE) != LM32_TWIPWM_CR_TDONE)
		;
}

void write_pwm(uint8_t value)
{
	writel(value, &tp->pwm);
}
