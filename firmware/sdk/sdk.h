#ifndef __SDK_H__
#define __SDK_H__

#include <stdint.h>

/* utils.S */
void irq_enable(void), irq_disable(void);
void irq_set_mask(uint32_t);
uint32_t irq_get_mask(void);
void jump(uint32_t target);	/* direct jump to code @target */
void halt(void);		/* endless loop */
uint32_t get_sp(void), get_gp(void);

/* intr.c */
void isr_register(int irq, void (*isr)(void));
void isr_unregister(int irq);

#endif
