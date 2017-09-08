/*
 * Author: Mikeqin <Fengling.Qin@gmail.com>
 *
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */

#ifndef __GPIO_H__
#define __GPIO_H__

#include <stdint.h>

#define GPIO_LED_RED	0xff0000
#define GPIO_LED_GREEN	0xff00
#define GPIO_LED_BLUE	0xff
#define GPIO_LED_WHITE	0xffffff
#define GPIO_LED_BLACK	0x0
#define GPIO_LED_YELLOW	0xffff00

void gpio_led(uint8_t value);
void gpio_ledr(uint8_t en);
void gpio_ledg(uint8_t en);
void gpio_ledb(uint8_t en);
void gpio_led_rgb(uint32_t rgb);

#endif	/* __GPIO_H__ */
