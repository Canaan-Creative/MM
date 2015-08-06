/*
 * Author: Xiangfu Liu <xiangfu@openmobilefree.net>
 * Bitcoin:	1CanaaniJzgps8EV6Sfmpb7T8RutpaeyFn
 *
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */

#ifndef _SYSTEM_CONFIG_H_
#define _SYSTEM_CONFIG_H_

#define IDLE_TIME	3	/* Seconds */
#define TEMP_TIME	1	/* Seconds */
#define TEST_TIME	15	/* Seconds */
#define IDLE_TEMP	65	/* Degree (C) */
#define TEST_CORE_COUNT	64	/* 4 * 16 */

#define FAN_00		0x3ff
#define FAN_10		0x399
#define FAN_20		0x333
#define FAN_50		0x1ff
#define FAN_100		0x000

/* run mode 4bit */
#define MOD_CUSTOM	0x0
#define MOD_ECO		0x1
#define MOD_NORMAL	0x2
#define MOD_TURBO	0x3

#define AVA4_DEFAULT_MODULES	64

#ifdef MM41
	#define ASIC_CORETEST_VOLT	0xa100 /* 0.7875V */
	#define ASIC_0V			0xff00
	#define ASIC_VOL_ECO		0x9100 /* 0.7625 */
	#define ASIC_VOL_NORMAL		0xc100 /* 0.8000 */
	#define ASIC_VOL_TURBO		0xfe00 /* 0.8250 */
#endif
#ifdef MM40
	#define ASIC_CORETEST_VOLT	0xce00 /* 0.7875V */
	#define ASIC_0V			0x8f00
	#define ASIC_VOL_ECO		0x8e00 /* 0.8000 */
	#define ASIC_VOL_NORMAL		0xf600 /* 0.8125 */
	#define ASIC_VOL_TURBO		0xb600 /* 0.8250 */
#endif
#ifdef MM50
	#define ASIC_CORETEST_VOLT	0xa100 /* 0.7875V */
	#define ASIC_0V			0xff00
	#define ASIC_VOL_ECO		0x9100 /* 0.7625 */
	#define ASIC_VOL_NORMAL		0xc100 /* 0.8000 */
	#define ASIC_VOL_TURBO		0xfe00 /* 0.8250 */
#endif

#if defined(MM40) || defined(MM41)
#define ASIC_FREQUENCY		200 /* MHz */
#define ASIC_PLL		0x1e278447
#endif
#ifdef MM50
#define ASIC_FREQUENCY		100 /* MHz */
#define ASIC_PLL		0x1e678447
#endif

#define ASIC_TIMEOUT_100M	0x4113e98

#if defined(MM40) || defined(MM41)
#define MINER_COUNT		10
#define ASIC_COUNT		4
#define SPI_SPEED		(0x2)
#endif

#ifdef MM50
#define MINER_COUNT		2
#define ASIC_COUNT		16
#define SPI_SPEED		(0x4)
#endif

#define PLL_COUNT		3

#define CPU_FREQUENCY		(50 * 1000 * 1000) /* 50Mhz */
#define UART_BAUD_RATE          (115200)

/* Interrupt
 * Define at last few lines of verilog/superkdf9/soc/superkdf9_simple.v
 */
#define IRQ_IIC			(0x00000004) /* 2 */
#define IRQ_UART		(0x00000008) /* 3 */
#define IRQ_UARTDEBUG		(0x00000010) /* 4 */
#define IRQ_TIMER0		(0x00000020) /* 5 */
#define IRQ_TIMER1		(0x00000040) /* 6 */

/* Registers */
#define UART0_BASE		(0x80000100)
#define UART1_BASE		(0x80000300)
#define SHA256_BASE		(0x80000400)
#define API_BASE		(0x80000500)
#define TWIPWM_BASE		(0x80000600)
#define SHIFTER_BASE0		(0x80000614) /* Blade 1 power controller */
#define TIMER_BASE		(0x80000620)
#define GPIO_BASE		(0x80000624)
#define CLKO_BASE		(0x80000628)
#define SHIFTER_BASE1		(0x8000062C) /* Blade 2 power controller */
#define SHIFTER_BASE2		(0x80000630) /* Front LED controller */
#define IIC_BASE		(0x80000700)
#define DNA_BASE		(0x80000710)
#define RBT_BASE		(0x80000714)
#define MBOOT_BASE		(0x80000800)

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
#define LM32_SHA256_CMD_RST	(1 << 2)
#define LM32_SHA256_CMD_DBL	(1 << 3)

struct lm32_sha256 {
	volatile unsigned int cmd;
	volatile unsigned int din;
	volatile unsigned int hash;
	volatile unsigned int hi;
	volatile unsigned int pre; /* Please read the A3255 datasheet */
};


/* API */
#define LM32_API_RET_LEN	11
struct lm32_api {
	volatile unsigned int tx;
	volatile unsigned int rx;
	volatile unsigned int state;
	volatile unsigned int timeout;
	volatile unsigned int sck;
	volatile unsigned int ram;
	volatile unsigned int lw;
	volatile unsigned int plla;
	volatile unsigned int pllc;
};

/* TWI PWM */
#define LM32_TWIPWM_CR_ENABLE	(1 << 0)
#define LM32_TWIPWM_CR_TSTART	(1 << 1)
#define LM32_TWIPWM_CR_TDONE	(1 << 2)
#define LM32_TWIPWM_CR_CMD_START	(0 << 4)
#define LM32_TWIPWM_CR_CMD_WD		(1 << 4)
#define LM32_TWIPWM_CR_CMD_RDACK	(2 << 4)
#define LM32_TWIPWM_CR_CMD_STOP		(3 << 4)
#define LM32_TWIPWM_CR_CMD_RCNOACKT	(4 << 4)

#define LM32_TWIPWM_WDG_ENABLE	(1 << 0);
/* [26:1] Feed Dog: 0x3~0x3ffffff;[WR] */

#define LM32_TWI_REG_TEMP0	0x48
#define LM32_TWI_REG_TEMP1	0x49

struct lm32_twipwm {
	volatile unsigned int cr; /* TWI ctrl register */
	volatile unsigned int wd; /* TWI write byte */
	volatile unsigned int rd; /* TWI read byte */
	volatile unsigned int pwm; /* PWM Counter register */
	volatile unsigned int wdg; /* Watch dog ctrl */
	volatile unsigned int pad; /* Watch dog ctrl */
	volatile unsigned int fan0; /* Watch dog ctrl */
	volatile unsigned int fan1; /* Watch dog ctrl */
};

struct lm32_shifter {
	volatile unsigned int reg; /* Shifter register for power chip */
};

struct lm32_timer {
	volatile unsigned int reg; /* Timer register */
};

struct lm32_gpio {
	volatile unsigned int reg; /* GPIO register */
};

struct lm32_clko {
	volatile unsigned int reg; /* CLK output register */
};

/* IIC */
#define LM32_IIC_CR_RX_CNT		(0x1FF)
#define LM32_IIC_CR_TX_CNT		(0x1FF << 9)
#define LM32_IIC_CR_WSTOP		(1 << 18)
#define LM32_IIC_CR_RSTOP		(1 << 19)
#define LM32_IIC_CR_RERR		(1 << 20)
#define LM32_IIC_CR_RXFIFORESET		(1 << 21)
#define LM32_IIC_CR_TXFIFORESET		(1 << 22)
#define LM32_IIC_CR_LOGICRESET		(1 << 23)
#define LM32_IIC_CR_RX_INTR_MASK_SET	(1 << 24)
#define LM32_IIC_CR_RX_INTR_MASK_CLEAR	(1 << 25)

struct lm32_iic {
	volatile unsigned int ctrl; /*ATWI ctrl*/
	volatile unsigned int addr; /*ATWI addr*/
	volatile unsigned int tx; /*ATWI tx*/
	volatile unsigned int rx; /*ATWI rx*/
};

/*DNA*/
#define LM32_DNA_CLK		(1 << 0)
#define LM32_DNA_DIN		(1 << 1)
#define LM32_DNA_READ		(1 << 2)
#define LM32_DNA_SHIFT		(1 << 3)
#define LM32_DNA_DOUT		(1 << 4)
#define LM32_DNA_MASK		(0x1f)

struct lm32_dna {
	volatile unsigned int dna;	/*DNA*/
};

struct lm32_rbt {
	volatile unsigned int rbt;	/*reboot*/
};

/* MBOOT */
#define LM32_MBOOT_WP_N         (1 << 0)
#define LM32_MBOOT_HOLD_N       (1 << 1)
#define LM32_MBOOT_MOSI(bit)    ((bit) << 2)
#define LM32_MBOOT_CS           (1 << 3)
#define LM32_MBOOT_SCL          (1 << 4)
#define LM32_MBOOT_MISO(bit)    ((bit) >> 5)
#define LM32_ICAP_WRITE         (1 << 16)
#define LM32_ICAP_CE            (1 << 17)
#define LM32_ICAP_CLK           (1 << 18)

struct lm32_mboot {
	volatile unsigned int flash;
	volatile unsigned int icap_o;
	volatile unsigned int icap_i;
};

#endif /* _SYSTEM_CONFIG_H_ */
