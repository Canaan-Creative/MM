/*
 * Author: Xiangfu Liu <xiangfu@openmobilefree.net>
 * Bitcoin:	1CanaaniJzgps8EV6Sfmpb7T8RutpaeyFn
 *
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */

#ifndef _SYSTEM_CONFIG_H_
#define _SYSTEM_CONFIG_H_

#define CPU_FREQUENCY		(50 * 1000 * 1000) /* 50Mhz */
#define UART_BAUD_RATE          (115200)

#define SPI_BASE		(0x80000000)
#define UART0_BASE		(0x80000100)
#define GPIO_BASE		(0x80000200)
#define UART1_BASE		(0x80000300)
#define SHA256_BASE		(0x80000400)
#define TASK_BASE		(0x80000500)


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

/* SHA256
 * Please read http://csrc.nist.gov/publications/fips/fips180-4/fips-180-4.pdf
 * Example here: http://csrc.nist.gov/groups/ST/toolkit/documents/Examples/SHA256.pdf
 * 1. Write the CMD_INIT
 * 2. Wait the CMD_DONE when write every 512bits data
 *    You may need pad the data to 512bits
 * 3. Wait teh CMD_DONE. read the result out
 */
#define LM32_SHA256_CMD_INIT	(1 << 0)
#define LM32_SHA256_CMD_DONE	(1 << 1)

struct lm32_sha256 {
	volatile unsigned int cmd;
	volatile unsigned int din;
	volatile unsigned int hash;
	volatile unsigned int hi;
	volatile unsigned int pre;
};


/* This is the ASIC task FIFO buffer registers */
#define LM32_TASK_STATE_TXFULL	(1 << 0)
#define LM32_TASK_STATE_FLUSH	(1 << 1)
#define LM32_TASK_STATE_TXCOUNT	(0x000000F0)
#define LM32_TASK_STATE_RXEMPTY	(1 << 16)
#define LM32_TASK_STATE_RXCOUNT (0x00F00000)

struct lm32_alink {
	volatile unsigned int tx;
	volatile unsigned int state;	/* Read only */
	volatile unsigned int en;
	volatile unsigned int busy;	/* Read only */
	volatile unsigned int rx;
};

#endif /* _SYSTEM_CONFIG_H_ */
