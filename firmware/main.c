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

#include "system_config.h"
#include "io.h"

struct lm32_uart *uart = (struct lm32_uart *)UART0_BASE;
struct lm32_sha256 *sha256 = (struct lm32_sha256 *)SHA256_BASE;

void uart_init(void)
{
	uint8_t value;
	/* Disable UART interrupts */
	writeb(0, &uart->ier);

	/* Line control 8 bit, 1 stop, no parity */
	writeb(LM32_UART_LCR_8BIT, &uart->lcr);

	/* Modem control, DTR = 1, RTS = 1 */
	writeb(LM32_UART_MCR_DTR | LM32_UART_MCR_RTS, &uart->mcr);

	/* Set baud rate */
	value = (CPU_FREQUENCY / UART_BAUD_RATE) & 0xff;
	writeb(value, &uart->divl);
	value = (CPU_FREQUENCY / UART_BAUD_RATE) >> 8;
	writeb(value, &uart->divh);
}

static int serial_tstc(void)
{
	if (readb(&uart->lsr) & LM32_UART_LSR_DR)
		return 1;

	return 0;
}

static unsigned char serial_getc()
{
	while (!serial_tstc())
		;

	return readb(&uart->rxtx);
}


static void serial_putc(const unsigned char c)
{
	if (c == '\n')
		serial_putc('\r');

	/* Wait for fifo to shift out some bytes */
	while (!((readb(&uart->lsr) & (LM32_UART_LSR_THRR | LM32_UART_LSR_TEMT)) == 0x60))
		;

	writeb(c, &uart->rxtx);
}

static void set_led(uint32_t led)
{
	volatile uint32_t *gpio_pio_data = (uint32_t *)GPIO_BASE;

	*gpio_pio_data = led;
}

static void delay(volatile uint32_t i)
{
	while (i--)
		;
}

const char *result = "\n{\"params\": ["
	"\"usrid\"," // miner id
	"\"263884\", " // job id
	"\"62000000\"," // extra nonce
	"\"5214f239\"," // ntime
	"\"26a8c33c\"" // nonce
	"],"
	"\"id\": 297,"
	"\"method\": \"mining.submit\"}\n";

const uint32_t sha256_in[16] = {
	0x61626380, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000018};

const uint32_t sha256_in2[32] = {
	0x61626364, 0x62636465, 0x63646566, 0x64656667,
	0x65666768, 0x66676869, 0x6768696A, 0x68696A6B,
	0x696A6B6C, 0x6A6B6C6D, 0x6B6C6D6E, 0x6C6D6E6F,
	0x6D6E6F70, 0x6E6F7071, 0x80000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x000001C0};

int main(void) {
	uint8_t tmp;
	uint32_t j, t32;

	uart_init();

	/* Test serial console */
	j = 0;
	while (result[j]) {
		serial_putc(result[j++]);
	}
	serial_putc('U');
	serial_putc('U');
	serial_putc('U');
	serial_putc('U');

	/* Test sha256 core: 1 block data*/
	writel(LM32_SHA256_CMD_INIT, &sha256->cmd);
	for (j = 0; j < 16; j++)
		writel(sha256_in[j], &sha256->in);
	while (!(readl(&sha256->cmd) & LM32_SHA256_CMD_DONE))
		;
	for (j = 0; j < 8; j++) {
		t32 = readl(&sha256->out);
		serial_putc((t32 & 0x000000ff) >> 0);
		serial_putc((t32 & 0x0000ff00) >> 8);
		serial_putc((t32 & 0x00ff0000) >> 16);
		serial_putc((t32 & 0xff000000) >> 24);
	}
	serial_putc('U');
	serial_putc('U');
	serial_putc('U');
	serial_putc('U');

	/* Test sha256 core: 2 block data*/
	writel(LM32_SHA256_CMD_INIT, &sha256->cmd);
	for (j = 0; j < 32; j++) {
		writel(sha256_in2[j], &sha256->in);
		if (!((j + 1) % 16))
			while (!(readl(&sha256->cmd) & LM32_SHA256_CMD_DONE))
				;
	}
	for (j = 0; j < 8; j++) {
		t32 = readl(&sha256->out);
		serial_putc((t32 & 0x000000ff) >> 0);
		serial_putc((t32 & 0x0000ff00) >> 8);
		serial_putc((t32 & 0x00ff0000) >> 16);
		serial_putc((t32 & 0xff000000) >> 24);
	}
	serial_putc('U');
	serial_putc('U');
	serial_putc('U');
	serial_putc('U');

	/* Test GPIO */
	j = 1;
	while (1) {
		delay(4000000);

		j++;
		set_led(0x00345678 | (j << 24));

		tmp = serial_getc();
		serial_putc(tmp);
	}

	return 0;
}

// vim: set ts=4 sw=4 fdm=marker :
