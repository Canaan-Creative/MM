/*
 * Author: Xiangfu Liu <xiangfu@openmobilefree.net>
 * Bitcoin:	1CanaaniJzgps8EV6Sfmpb7T8RutpaeyFn
 *
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */

#ifndef _IO_H_
#define _IO_H_

#ifndef bswap_16
#define	bswap_16(value)  \
	((((value) & 0xff) << 8) | ((value) >> 8))

#define	bswap_32(value)	\
	(((uint32_t)bswap_16((uint16_t)((value) & 0xffff)) << 16) | \
	(uint32_t)bswap_16((uint16_t)((value) >> 16)))

#define	bswap_64(value)	\
	(((uint64_t)bswap_32((uint32_t)((value) & 0xffffffff)) \
	    << 32) | \
	(uint64_t)bswap_32((uint32_t)((value) >> 32)))
#endif

#define readb(addr) (*(volatile unsigned char *)(addr))
#define readw(addr) (*(volatile unsigned short *)(addr))
#define readl(addr) (*(volatile unsigned int *)(addr))

#define writeb(b,addr) (*(volatile unsigned char *)(addr)) = (b)
#define writew(b,addr) (*(volatile unsigned short *)(addr)) = (b)
#define writel(b,addr) (*(volatile unsigned int *)(addr)) = (b)

#define likely(x)	(x)
#define unlikely(x)	(x)

#define ARRAY_SIZE(x)	(sizeof(x) / sizeof((x)[0]))

#endif /* _IO_H_ */
