/*
 * Author: Xiangfu Liu <xiangfu@openmobilefree.net>
 * Bitcoin:	1CanaaniJzgps8EV6Sfmpb7T8RutpaeyFn
 *
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */

#ifndef _DEFINES_H_
#define _DEFINES_H_

#define DEBUG 1
#ifdef DEBUG
void hexdump(const uint8_t *p, unsigned int len);

#include "uart.h"
char printf_buf32[32];
#define debug32(...)	do {				\
		m_sprintf(printf_buf32, __VA_ARGS__);	\
		uart1_puts(printf_buf32);		\
	} while(0)
#else
#define debug32(...)
#define hexdump(x, y)
#endif


#endif	/* _DEFINES_H_ */
