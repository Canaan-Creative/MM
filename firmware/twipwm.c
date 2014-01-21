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

void twi_write_2byte(uint16_t buf, uint8_t addr)
{
	twi_start();
	twi_write(addr << 1); /* slave addr */
	twi_write(0x00);	/* register addr */
	twi_write(buf);
	twi_write(buf >> 8);
	twi_stop();
}

uint16_t twi_read_2byte(uint8_t addr)
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

void write_pwm(uint32_t value)
{
	writel(value, &tp->pwm);
}

void wdg_init(int enable)
{
	writel(enable, &tp->wdg);
}

void wdg_feed(uint32_t value)
{
	writel(((value & 0x3ffffff) << 1), &tp->wdg);
}

void reset()
{
	wdg_feed(8);
}

uint32_t read_fan0()
{
	return readl(&tp->fan0) * 30;
}

uint32_t read_fan1()
{
	return readl(&tp->fan1) * 30;
}

void adjust_fan(uint32_t pwm)
{
	static uint32_t value = 0x3ff;

	if (value == pwm)
		return;

	value = pwm;
	if (value < 0)
		value = 0;
	if (value > 0x3ff)
		value = 0x3ff;

	write_pwm(value);
}

uint16_t read_temp0()
{
	return (twi_read_2byte(LM32_TWI_REG_TEMP0) >> 4) / 16;
}

uint16_t read_temp1()
{
	return (twi_read_2byte(LM32_TWI_REG_TEMP0) >> 4) / 16;
}
