/*
 * Author: Xiangfu Liu <xiangfu@openmobilefree.net>
 * Bitcoin:	1CanaaniJzgps8EV6Sfmpb7T8RutpaeyFn
 *
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */

#ifndef _TWI_H_
#define _TWI_H_

void twi_write_2byte(uint16_t buf, uint8_t addr);
uint16_t twi_read_2byte(uint8_t addr);

void write_pwm(uint8_t value);

void wdg_init(int enable);
void wdg_feed(uint32_t value);

uint32_t read_fan0();
uint32_t read_fan1();

uint16_t read_temp0();
uint16_t read_temp1();

void adjust_fan(uint32_t pwm);
void reset();

#endif	/* _TWI_H_ */
