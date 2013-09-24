//
// Author: Minux
// Author: Xiangfu Liu <xiangfu@openmobilefree.net>
// Bitcoin:	1CanaaniJzgps8EV6Sfmpb7T8RutpaeyFn
//
// This is free and unencumbered software released into the public domain.
// For details see the UNLICENSE file at the root of the source tree.
//

#include "sdk.h"
#include "minilibc.h"

void setled(uint32_t led) {
	int t;
	asm volatile ("user %0, %1, %1, 0x1": "=r"(t) : "r"(led));
}

void delay(volatile uint32_t i) {
	while (i--)
		;
}

int main(void) {
	int i = 0, j;
	unsigned int *gpio_pio_data = (unsigned int *)0x80000200;
	unsigned int *uart0_buf = (unsigned int *)0x80000100;
	j = 1;
	while (1) {
		delay(16000000);
		j++;
		*gpio_pio_data = 0x00345678|(j<<24) ;
		*uart0_buf = 0x12345678 ;
	}
	while (1) {
		asm volatile ("rcsr    %0, IP": "=r"(i));
		asm volatile ("user %0, %1, %1, 0xb": "=r"(j) : "r"(i));
	}
	while (1) {
		asm volatile ("user %0, %1, %1, 0x55" : "=r"(j) : "r"(i));
		if (j != i + i) break;
		asm volatile ("user %0, %1, %1, 0xaa" : "=r"(j) : "r"(~i));
		if (j != ~i + ~i) break;
		i++;
	}
	while (1)
		;
	return(0);
}

// vim: set ts=4 sw=4 fdm=marker :
