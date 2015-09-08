/*
 * Author: Xiangfu Liu <xiangfu@openmobilefree.net>
 * Bitcoin:	1CanaaniJzgps8EV6Sfmpb7T8RutpaeyFn
 *
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */

#include <stdint.h>
#include "minilibc/minilibc.h"
#include "system_config.h"
#include "defines.h"
#include "io.h"
#include "twipwm.h"
#include "timer.h"

static struct lm32_twipwm *tp = (struct lm32_twipwm *)TWIPWM_BASE;
static uint8_t g_lcd_bg = 0;
static uint8_t g_lcd_dispctrl = 0;

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

static void twi_write_byte(uint8_t addr, uint8_t byte)
{
	twi_start();
	twi_write(addr << 1);
	twi_write(byte);
	twi_stop();
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
	int16_t neg = 1;
	uint16_t hex_tmp;

	if (timer_read(1))
		return last;

	timer_set(1, TEMP_TIME);

	memcpy(temp, temp + 1, 9 * sizeof(int16_t));
	hex_tmp = twi_read_2byte(LM32_TWI_REG_TEMP1) >> 4;
	if (hex_tmp > 0x7ff) {
		hex_tmp = ((~(hex_tmp & 0x7ff)) & 0x7ff) + 1;
		neg = -1;
	}
	temp[9] = (hex_tmp / 16) * neg;
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

static void lcd_op(uint8_t data, uint8_t mode, uint8_t rs)
{
	/* write command or high 8 bits data */
	twi_write_byte(LM32_TWI_REG_LCD, (data & 0xf0) | g_lcd_bg | rs);
	/* enable command */
	twi_write_byte(LM32_TWI_REG_LCD, (data & 0xf0) | g_lcd_bg | rs | LM32_TWIPWM_LCD_EN);
	delayus(50);
	twi_write_byte(LM32_TWI_REG_LCD, (data & 0xf0) | g_lcd_bg | rs);
	delayus(800);

	if (mode == LM32_TWIPWM_LCD_FULLMODE) {
		/* write low 8bit */
		twi_write_byte(LM32_TWI_REG_LCD, ((data << 4) & 0xf0) | g_lcd_bg | rs);
		/* enable command */
		twi_write_byte(LM32_TWI_REG_LCD, ((data << 4) & 0xf0) | g_lcd_bg | rs | LM32_TWIPWM_LCD_EN);
		delayus(50);
		twi_write_byte(LM32_TWI_REG_LCD, ((data << 4) & 0xf0) | g_lcd_bg | rs);
		delayus(800);
	}
}

/* https://www.sparkfun.com/datasheets/LCD/HD44780.pdf */
void lcd_init(void)
{
	uint8_t i;

	/* Initializing HD44780.pdf P.45 */
	lcd_op(0, LM32_TWIPWM_LCD_HALFMODE, 0);
	delay(50);

	for (i = 0; i < 3; i++) {
		lcd_op(0x30, LM32_TWIPWM_LCD_HALFMODE, 0);
		delay(5);
	}

	/* set to 4-Bit Interface */
	lcd_op(0x20, LM32_TWIPWM_LCD_HALFMODE, 0);

	/* set line, dot */
	lcd_op(LM32_TWIPWM_LCD_FUNC | LM32_TWIPWM_LCD_4BIT | LM32_TWIPWM_LCD_2LINE | LM32_TWIPWM_LCD_5X8DOTS, LM32_TWIPWM_LCD_FULLMODE, 0);
	/* set display, turn off cursor and blink */
	g_lcd_dispctrl = LM32_TWIPWM_LCD_DISP | LM32_TWIPWM_LCD_DISPON | LM32_TWIPWM_LCD_CURSOROFF | LM32_TWIPWM_LCD_BLINKOFF;
	lcd_op(g_lcd_dispctrl, LM32_TWIPWM_LCD_FULLMODE, 0);

	/* clear */
	lcd_op(LM32_TWIPWM_LCD_CLEAR, LM32_TWIPWM_LCD_FULLMODE, 0);

	/* entry mode */
	lcd_op(LM32_TWIPWM_LCD_MODE | LM32_TWIPWM_LCD_ENTRYL | LM32_TWIPWM_LCD_ENTRYDEC, LM32_TWIPWM_LCD_FULLMODE, 0);

	/* home */
	lcd_op(LM32_TWIPWM_LCD_HOME, LM32_TWIPWM_LCD_FULLMODE, 0);
}

void lcd_on(void)
{
	g_lcd_bg = LM32_TWIPWM_LCD_BGON;
	g_lcd_dispctrl |= LM32_TWIPWM_LCD_DISPON;
	lcd_op(g_lcd_dispctrl, LM32_TWIPWM_LCD_FULLMODE, 0);
}

void lcd_off(void)
{
	g_lcd_bg = ~LM32_TWIPWM_LCD_BGON;
	g_lcd_dispctrl &= ~LM32_TWIPWM_LCD_DISPON;
	lcd_op(g_lcd_dispctrl, LM32_TWIPWM_LCD_FULLMODE, 0);
}

void lcd_clear(void)
{
	lcd_op(LM32_TWIPWM_LCD_CLEAR, LM32_TWIPWM_LCD_FULLMODE, 0);
	delay(2);
}

void lcd_home(void)
{
	lcd_op(LM32_TWIPWM_LCD_HOME, LM32_TWIPWM_LCD_FULLMODE, 0);
	delay(2);
}

void lcd_setcursor(uint8_t col, uint8_t row)
{
	int row_offsets[] = {0x00, 0x40, 0x14, 0x54};

	/* 16 x 2 */
	if (row > 2)
		row = 1;

	lcd_op(LM32_TWIPWM_LCD_DDIR | (col + row_offsets[row]), LM32_TWIPWM_LCD_FULLMODE, 0);
}

void lcd_leftscroll(void)
{
	lcd_op(LM32_TWIPWM_LCD_SHIFT | LM32_TWIPWM_LCD_DISPMOVE | LM32_TWIPWM_LCD_SL, LM32_TWIPWM_LCD_FULLMODE, 0);
}

void lcd_rightscroll(void)
{
	lcd_op(LM32_TWIPWM_LCD_SHIFT | LM32_TWIPWM_LCD_DISPMOVE | LM32_TWIPWM_LCD_SR, LM32_TWIPWM_LCD_FULLMODE, 0);
}

void lcd_write(char c)
{
	lcd_op(c, LM32_TWIPWM_LCD_FULLMODE, LM32_TWIPWM_LCD_RS);
}

void lcd_puts(const char *s)
{
	while (*s)
		lcd_write(*s++);
}

