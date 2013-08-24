#include <sdk.h>
#include <minilibc.h>

void
setled(uint32_t led) {
	int t;
	asm volatile ("user %0, %1, %1, 0x1": "=r"(t) : "r"(led));
}

void
delay(volatile uint32_t i) {
	while (i--)
		;
}

int
main(void) {
	int i = 0, j;
	//irq_set_mask(0x1);
	//irq_enable();
	j = 1;
	while (1) {
		setled(j);
		delay(16000000);
		j <<= 1;
		if (j > (1 << 3))
			j = 1;
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
