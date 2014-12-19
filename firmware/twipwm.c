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
#include "twipwm.h"
#include "timer.h"

static struct lm32_twipwm *tp = (struct lm32_twipwm *)TWIPWM_BASE;

static void twi_start(void)
{
	writel(LM32_TWIPWM_CR_ENABLE | LM32_TWIPWM_CR_TSTART, &tp->cr);
	while((readl(&tp->cr) & LM32_TWIPWM_CR_TDONE) != LM32_TWIPWM_CR_TDONE)
		;
}

static void twi_write(uint8_t value)
{
	writel(value, &tp->wd);
	writel(LM32_TWIPWM_CR_ENABLE | LM32_TWIPWM_CR_TSTART | LM32_TWIPWM_CR_CMD_WD
	       , &tp->cr);
	while((readl(&tp->cr) & LM32_TWIPWM_CR_TDONE) != LM32_TWIPWM_CR_TDONE)
		;
}

static uint32_t twi_read(void)
{
	writel(LM32_TWIPWM_CR_ENABLE | LM32_TWIPWM_CR_TSTART | LM32_TWIPWM_CR_CMD_RDACK
	       , &tp->cr);
	while((readl(&tp->cr) & LM32_TWIPWM_CR_TDONE) != LM32_TWIPWM_CR_TDONE)
		;

	return readl(&tp->rd);
}

static void twi_stop(void)
{
	writel(LM32_TWIPWM_CR_ENABLE | LM32_TWIPWM_CR_TSTART | LM32_TWIPWM_CR_CMD_STOP
	       , &tp->cr);
	while((readl(&tp->cr) & LM32_TWIPWM_CR_TDONE) != LM32_TWIPWM_CR_TDONE)
		;
}

static uint16_t twi_read_2byte(uint8_t addr)
{
	uint32_t tmp;
	twi_start();
	twi_write(addr << 1);	/* slave addr */
	twi_write(0x00);	/* register addr */
	twi_stop();
	twi_start();
	twi_write((addr << 1) | 0x1);/* slave addr + read */
	tmp = twi_read();
	tmp = (tmp << 8) | twi_read();
	twi_stop();
	return (tmp & 0xffff);
}

static void write_pwm(uint32_t value)
{
	writel(value, &tp->pwm);
}

void wdg_feed(uint32_t value)
{
	writel(((value & 0x7fffffff) << 1) | 1, &tp->wdg);
}

uint32_t read_fan(void)
{
	return readl(&tp->fan0) * 30;
}

void adjust_fan(uint32_t pwm)
{
	static uint32_t value = 0x3ff;

	if (value == pwm)
		return;

	value = pwm;

	if (value > 0x3ff)
		value = 0x3ff;

	write_pwm(value);
}

int16_t read_temp(void)
{
	static int16_t temp[10];
	static int16_t last;
	int i;
	int32_t sum = 0;
	int16_t min;
	int16_t max;

	if (timer_read(1))
		return last;

	timer_set(1, TEMP_TIME);

	memcpy(temp, temp + 1, 9 * sizeof(int16_t));
	temp[9] = (twi_read_2byte(LM32_TWI_REG_TEMP1) >> 4) / 16;
	min = max = temp[9];

	for (i = 0; i < 10; i++) {
		if(max < temp[i])
			max = temp[i];
		if(min > temp[i])
			min = temp[i];
		sum = sum + temp[i];
	}

	last = (int16_t)((sum - max - min) / 8);
	return last;
}
