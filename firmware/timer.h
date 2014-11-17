/*
 * Author: Xiangfu Liu <xiangfu@openmobilefree.net>
 * Bitcoin:	1CanaaniJzgps8EV6Sfmpb7T8RutpaeyFn
 *
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */

#ifndef __TIMER_H__
#define __TIMER_H__

#include <stdint.h>

void timer0_isr(void);
void timer1_isr(void);
void timer_mask_set(unsigned char timer);
void timer_mask_clean(unsigned char timer);
void timer_set(unsigned char timer, unsigned char load);
uint32_t timer_read(unsigned char timer);

void gpio_reset_asic(void);
int read_power_good(void);

int clko_init(uint32_t value);

void gpio_led(uint8_t value);

#endif	/* __TIMER_H__ */
