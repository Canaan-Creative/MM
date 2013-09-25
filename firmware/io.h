/*
 * Author: Xiangfu Liu <xiangfu@openmobilefree.net>
 * Bitcoin:	1CanaaniJzgps8EV6Sfmpb7T8RutpaeyFn
 *
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */

#ifndef _ASM_IO_H
#define _ASM_IO_H

#define readb(addr) (*(volatile unsigned char *)(addr))
#define readw(addr) (*(volatile unsigned short *)(addr))
#define readl(addr) (*(volatile unsigned int *)(addr))

#define writeb(b,addr) (*(volatile unsigned char *)(addr)) = (b)
#define writew(b,addr) (*(volatile unsigned short *)(addr)) = (b)
#define writel(b,addr) (*(volatile unsigned int *)(addr)) = (b)

#endif /* _ASM_IO_H */
