#include <sdk.h>
#include <minilibc.h>

int
main(void) {
	int i = 0, j;
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
