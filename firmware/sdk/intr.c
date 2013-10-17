/* IRQ handling */
#include "sdk.h"

#define ISR_TABLE_LEN	(32)

static void isr_null(void)
{
}

static void (*isr_table[ISR_TABLE_LEN])(void);

void irq_handler(uint32_t pending)
{
	int i, j;

	for (i = 0; i < ISR_TABLE_LEN; i++) {
		if (pending & 0x01)
			(*isr_table[i])();
		pending >>= 1;
		asm volatile ("user %0, %1, %1, 0x0f" : "=r"(j) : "r"(i));
	}
}

void isr_init(void)
{
	int i;
	for (i = 0; i < ISR_TABLE_LEN; i++)
		isr_table[i] = &isr_null;
}

void isr_register(int irq, void (*isr)(void))
{
	isr_table[irq] = isr;
}

void isr_unregister(int irq)
{
	isr_table[irq] = &isr_null;
}
