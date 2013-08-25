#ifndef __SDK_H__
#define __SDK_H__

// common types
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;
typedef long long int64_t;
typedef unsigned long long uint64_t;

// utils.S
void irq_enable(void), irq_disable(void);
void irq_set_mask(uint32_t);
uint32_t irq_get_mask(void);
void jump(uint32_t target); // direct jump to code @target
void halt(void); // endless loop
uint32_t get_sp(void), get_gp(void);

// intr.c
void isr_register(int irq, void (*isr)(void));
void isr_unregister(int irq);

#endif
