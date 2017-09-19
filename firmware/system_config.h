/*
 * Author: Mikeqin <Fengling.Qin@gmail.com>
 *
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */

#ifndef _SYSTEM_CONFIG_H_
#define _SYSTEM_CONFIG_H_

#define IDLE_TIME	3

#define CPU_FREQUENCY_MHZ	(50)
#define CPU_FREQUENCY		(CPU_FREQUENCY_MHZ * 1000 * 1000) /* 50Mhz */
#define XCLK_FREQUENCY		(25 * 1000 * 1000)
#define UART_BAUD_RATE          (115200)

/* Interrupt
 * Define at last few lines of verilog/superkdf9/soc/superkdf9_simple.v
 */
#define IRQ_UART		(0x00000008) /* 3 */
#define IRQ_UARTDEBUG		(0x00000010) /* 4 */

#define IRQ_TIMER0		(0x00000020) /* 5 */
#define IRQ_TIMER1		(0x00000040) /* 6 */
#define IRQ_UART1		(0x00000080) /* 7 */

/* Registers */
#define UART0_BASE		(0x80000100)
#define UART1_BASE		(0x80000200)
#define UART2_BASE		(0x80000300)
#define TIMER_BASE		(0x80000620)
#define GPIO_BASE		(0x80000624)

/* UART */
#define LM32_UART_IER_RBRI	(1 << 0)
#define LM32_UART_IER_THRI	(1 << 1)

#define LM32_UART_STAT_RX_EVT	(0x4)
#define LM32_UART_STAT_TX_EVT	(0x2)

/* Line status register */
#define LM32_UART_LSR_DR	(1 << 0)
#define LM32_UART_LSR_THRR	(1 << 5)
#define LM32_UART_LSR_TEMT	(1 << 6)

/* Line control register */
#define LM32_UART_LCR_WLS0	(1 << 0)
#define LM32_UART_LCR_WLS1	(1 << 1)
#define LM32_UART_LCR_8BIT	(LM32_UART_LCR_WLS1 | LM32_UART_LCR_WLS0)

/* Modem control register */
#define LM32_UART_MCR_DTR	(1 << 0)
#define LM32_UART_MCR_RTS	(1 << 1)

struct lm32_uart {
	volatile unsigned char rxtx;
	volatile unsigned char ier;
	volatile unsigned char iir;
	volatile unsigned char lcr;
	volatile unsigned char mcr;
	volatile unsigned char lsr;
	volatile unsigned char msr;
	volatile unsigned char pad0;
	volatile unsigned char divl;
	volatile unsigned char divh;
};

struct lm32_timer {
	volatile unsigned int reg; /* Timer register */
};

struct lm32_gpio {
	volatile unsigned int reg; /* GPIO register */
};

#endif /* _SYSTEM_CONFIG_H_ */
