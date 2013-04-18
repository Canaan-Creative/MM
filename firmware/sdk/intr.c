// IRQ handling
#include <sdk.h>

static void 
isr_null(void) {
}

static void (*isr_table[32])(void);

void
irq_handler(uint32_t pending) {
	int i;

	for (i=0; i<32; i++) {
		if (pending & 0x01) (*isr_table[i])();
		pending >>= 1;
	}
}

void
isr_init(void) {
	int i;
	for (i=0; i<32; i++)
		isr_table[i] = &isr_null;
}

void
isr_register(int irq, void (*isr)(void)) {
	isr_table[irq] = isr;
}

void
isr_unregister(int irq) {
	isr_table[irq] = &isr_null;
}
