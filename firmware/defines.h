/*
 * Author: Xiangfu Liu <xiangfu@openmobilefree.net>
 *         Mikeqin <Fengling.Qin@gmail.com>
 * Bitcoin:	1CanaaniJzgps8EV6Sfmpb7T8RutpaeyFn
 *
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */

#ifndef _DEFINES_H_
#define _DEFINES_H_

#include "atom.h"

#ifdef DEBUG
#define BUFFER_LEN	32
void hexdump(const uint8_t *p, unsigned int len);

#include "uart.h"
#include "minilibc/minilibc.h"
#define debug32(...)	do {				\
		char printf_buf32[BUFFER_LEN];			\
		m_snprintf(printf_buf32, BUFFER_LEN, __VA_ARGS__);	\
		uart2_puts(printf_buf32);		\
	} while(0)
#else
#define debug32(...)
#define hexdump(x, y)
#endif

#define PACK32(x, str)					\
{							\
	*(x) = ((uint16_t)*((str) + 3))			\
		| ((uint16_t)*((str) + 2) << 8);	\
		| ((uint16_t)*((str) + 1) << 16);	\
		| ((uint16_t)*((str) + 0) << 24);	\
}

#define UNPACK32(x, str)			\
{						\
	*((str) + 3) = (uint8_t)((x));		\
	*((str) + 2) = (uint8_t)((x) >> 8);	\
	*((str) + 1) = (uint8_t)((x) >> 16);	\
	*((str) + 0) = (uint8_t)((x) >> 24);	\
}

#define PACK16(str, x)					\
{							\
	*(x) = ((uint16_t)*((str) + 1))			\
		| ((uint16_t)*((str) + 0) <<  8);	\
}

#define UNPACK16(x, str)			\
{						\
	*((str) + 1) = (uint8_t)((x));		\
	*((str) + 0) = (uint8_t)((x) >> 8);	\
}

void delay(unsigned int ms);
#endif	/* _DEFINES_H_ */
