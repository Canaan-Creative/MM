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

<<<<<<< HEAD
//`define SHA256_TEST		"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"
//`define SHA256_TEST_PADDING	{1'b1,63'b0,448'b0,64'd448}	// 448 bit
//`define SHA256_TEST_RESULT	256'h248d6a61_d20638b8_e5c02693_0c3e6039_a33ce459_64ff2167_f6ecedd4_19db06c1
void
setled(uint32_t led) {
	int t;
	asm volatile ("user %0, %1, %1, 0x1": "=r"(t) : "r"(led));
}

void
delay(volatile uint32_t i) {
=======
static void delay(volatile uint32_t i) {
>>>>>>> 0a89e8ab864edff626cf02fb1fc245db3fcab4c8
	while (i--)
		;
}

<<<<<<< HEAD
unsigned int data[32] = {
  0x61626364
, 0x62636465
, 0x63646566
, 0x64656667
, 0x65666768
, 0x66676869
, 0x6768696a
, 0x68696a6b
, 0x696a6b6c
, 0x6a6b6c6d
, 0x6b6c6d6e
, 0x6c6d6e6f
, 0x6d6e6f70
, 0x6e6f7071
, 0x80000000
, 0x00000000
, 0x00000000
, 0x00000000
, 0x00000000
, 0x00000000
, 0x00000000
, 0x00000000
, 0x00000000
, 0x00000000
, 0x00000000
, 0x00000000
, 0x00000000
, 0x00000000
, 0x00000000
, 0x00000000
, 0x00000000
, 0x000001c0
} ;
int
main(void) {
	int i = 0, j;
	unsigned int *gpio_pio_data = (unsigned int *)0x80000200 ; 
	volatile unsigned int tmp ;
	volatile unsigned int *sha_cmd = (unsigned int *)0x80000400 ; 
	volatile unsigned int *sha_din = (unsigned int *)0x80000404 ; 
	volatile unsigned int *sha_hash = (unsigned int *)0x80000408 ; 
	//unsigned int *uart0_buf = ( unsigned int *)0x80000100 ; 
	//unsigned int *gpio_pio_tri  = (unsigned int *)0x80000204 ; 
	//irq_set_mask(0x1);
	//irq_enable();
	j = 1;i=1;
//	for( i=0 ; i<16 ;i++)
//		*sha_din = i ;
//	while( ((*sha_cmd) & 0x2) != 0x2 ) ;
//	for( i=16 ; i<32 ;i++)
//		*sha_din = i ;
//	while( ((*sha_cmd) & 0x2) != 0x2 ) ;
//	for( i=0 ; i<8 ;i++)
//		tmp = *sha_hash ;
//
=======
int main(void) {
	int i = 0, j;
	unsigned int *gpio_pio_data = (unsigned int *)0x80000200;
	unsigned int *uart0_buf = (unsigned int *)0x80000100;
	j = 1;
>>>>>>> 0a89e8ab864edff626cf02fb1fc245db3fcab4c8
	while (1) {
		delay(16000000);
		j++;
		*gpio_pio_data = 0x00345678|(j<<24) ;
<<<<<<< HEAD
		*sha_cmd = 0x1 ;
		for(i=0;i<16;i++)
			*sha_din = data[i] ;
		tmp = *sha_cmd ;
		while( (tmp & 0x2) != 0x2 )
			tmp = *sha_cmd ;
		*sha_cmd = 0x0 ;

		for(i=16;i<32;i++)
			*sha_din = data[i] ;
		tmp = *sha_cmd ;
		while( (tmp & 0x2) != 0x2 )
			tmp = *sha_cmd ;
		for( i=0 ; i<8 ;i++)
			tmp = *sha_hash ;
	
		//*uart0_buf = 0x12345678 ;
		//j <<= 1;
		//if (j > (1 << 3))
		//	j = 1;
=======
		*uart0_buf = 0x12345678 ;
>>>>>>> 0a89e8ab864edff626cf02fb1fc245db3fcab4c8
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
