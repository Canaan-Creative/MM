/*
 * Author: Xiangfu Liu <xiangfu@openmobilefree.net>
 * Bitcoin:	1CanaaniJzgps8EV6Sfmpb7T8RutpaeyFn
 *
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */

#ifndef _TWI_H_
#define _TWI_H_

void twi_start(void);
void twi_write(uint8_t value);
uint8_t twi_read(void);
void twi_stop(void);
void write_pwm(uint8_t value);

#endif	/* _TWI_H_ */
