/*
 * Author: Mikeqin <Fengling.Qin@gmail.com>
 *
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */

#include "sdk/intr.h"
#include "system_config.h"
#include "defines.h"
#include "io.h"
#include "gpio.h"

static struct lm32_gpio *gpio = (struct lm32_gpio *)GPIO_BASE;
static uint32_t g_io;

void gpio_led(uint8_t led)
{
	g_io &= ~(1 << 8);
	g_io |= (led & 1) << 8;
	writel(g_io, &gpio->reg);
}

void gpio_ledr(uint8_t en)
{
	g_io &= ~(1 << 10);
	en = !en;
	g_io |= ((en & 1) << 10);
	writel(g_io, &gpio->reg);
}

void gpio_ledg(uint8_t en)
{
	g_io &= ~(1 << 9);
	en = !en;
	g_io |= ((en & 1) << 9);
	writel(g_io, &gpio->reg);
}

void gpio_ledb(uint8_t en)
{
	g_io &= ~(1 << 11);
	en = !en;
	g_io |= ((en & 1) << 11);
	writel(g_io, &gpio->reg);
}

void gpio_led_rgb(uint32_t rgb)
{
	if (rgb & GPIO_LED_BLUE)
		gpio_ledb(1);
	else
		gpio_ledb(0);

	if (rgb & GPIO_LED_GREEN)
		gpio_ledg(1);
	else
		gpio_ledg(0);

	if (rgb & GPIO_LED_RED)
		gpio_ledr(1);
	else
		gpio_ledr(0);
}

