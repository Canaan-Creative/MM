/*
 * Author: Xiangfu Liu <xiangfu@openmobilefree.net>
 * Bitcoin:	1CanaaniJzgps8EV6Sfmpb7T8RutpaeyFn
 *
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */

#ifndef _IO_H_
#define _IO_H_

#ifndef bswap_32
#define	bswap_32(value)	\
	((value >> 24) | (value << 24) | ((value >> 8) & 0xff00) | ((value << 8) & 0xff0000))
#endif

#define readb(addr) (*(volatile unsigned char *)(addr))
#define readw(addr) (*(volatile unsigned short *)(addr))
#define readl(addr) (*(volatile unsigned int *)(addr))

#define writeb(b,addr) (*(volatile unsigned char *)(addr)) = (b)
#define writew(b,addr) (*(volatile unsigned short *)(addr)) = (b)
#define writel(b,addr) (*(volatile unsigned int *)(addr)) = (b)

#define ARRAY_SIZE(x)	(sizeof(x) / sizeof((x)[0]))

#endif /* _IO_H_ */
