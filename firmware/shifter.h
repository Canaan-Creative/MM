/*
 * Author: Xiangfu Liu <xiangfu@openmobilefree.net>
 * Bitcoin:	1CanaaniJzgps8EV6Sfmpb7T8RutpaeyFn
 *
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */

#ifndef __SHIFTER_H__
#define __SHIFTER_H__

#include <stdint.h>

void set_voltage(uint32_t value);
uint32_t get_voltage();

#endif	/* __SHIFTER_H__ */
