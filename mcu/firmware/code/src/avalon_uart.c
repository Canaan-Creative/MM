/*
 * @brief
 *
 * @note
 * Author: Mikeqin Fengling.Qin@gmail.com
 *
 * @par
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */
#include <string.h>
#include "board.h"

#include "protocol.h"
#include "defines.h"
#include "avalon_uart.h"

/* Ring buffer count (default size: AVAM_P_COUNT) */
#define UART_RX_BUF_CNT	2
#define UART_TX_BUF_CNT	2
#define UART_RX_BUF_SZ (UART_RX_BUF_CNT * AVAM_P_COUNT)
#define UART_TX_BUF_SZ (UART_TX_BUF_CNT * AVAM_P_COUNT)

static RINGBUFF_T uart_rxrb, uart_txrb;
static uint8_t uart_rxdata[UART_RX_BUF_SZ], uart_txdata[UART_TX_BUF_SZ];

static void init_uart_pinmux(void)
{
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 18, IOCON_FUNC1 | IOCON_MODE_INACT);	/* PIO0_18 used for RXD */
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 19, IOCON_FUNC1 | IOCON_MODE_INACT);	/* PIO0_19 used for TXD */
}

void uart_init(void)
{
	/* Board specific muxing */
	init_uart_pinmux();

	Chip_UART_Init(LPC_USART);
	Chip_UART_SetBaudFDR(LPC_USART, 115200);
	Chip_UART_ConfigData(LPC_USART, (UART_LCR_WLEN8 | UART_LCR_SBS_1BIT));
	Chip_UART_SetupFIFOS(LPC_USART, (UART_FCR_FIFO_EN | UART_FCR_TRG_LEV2));
	Chip_UART_TXEnable(LPC_USART);

	RingBuffer_Init(&uart_rxrb, uart_rxdata, 1, UART_RX_BUF_SZ);
	RingBuffer_Init(&uart_txrb, uart_txdata, 1, UART_TX_BUF_SZ);

	/* Enable receive data and line status interrupt */
	Chip_UART_IntEnable(LPC_USART, (UART_IER_RBRINT | UART_IER_RLSINT));

	/* Enable Interrupt for UART channel */
	/* Priority = 1 */
	NVIC_SetPriority(UART0_IRQn, 1);
	/* Enable Interrupt for UART channel */
	NVIC_EnableIRQ(UART0_IRQn);
}

void UART_IRQHandler(void)
{
	Chip_UART_IRQRBHandler(LPC_USART, &uart_rxrb, &uart_txrb);
}

/* Gets current read count. */
uint32_t uart_rxrb_cnt(void)
{
	return RingBuffer_GetCount(&uart_rxrb);
}

/* Read data from uart */
uint32_t uart_read(uint8_t *pbuf, uint32_t buf_len)
{
	uint16_t cnt = 0;

	if (pbuf)
		cnt = Chip_UART_ReadRB(LPC_USART, &uart_rxrb, pbuf, buf_len);

	return cnt;
}

/* Send data to uart */
uint32_t uart_write(uint8_t *pbuf, uint32_t len)
{
	uint32_t ret = 0;

	if (pbuf)
		ret = Chip_UART_SendRB(LPC_USART, &uart_txrb, pbuf, len);

	return ret;
}

/* clear UART tx ringbuffer */
void uart_flush_txrb(void)
{
	Chip_UART_SetupFIFOS(LPC_USART, (UART_FCR_FIFO_EN | UART_FCR_TRG_LEV2 | UART_FCR_TX_RS));
	RingBuffer_Flush(&uart_txrb);
}

/* clear UART rx ringbuffer */
void uart_flush_rxrb(void)
{
	Chip_UART_SetupFIFOS(LPC_USART, (UART_FCR_FIFO_EN | UART_FCR_TRG_LEV2 | UART_FCR_RX_RS));
	RingBuffer_Flush(&uart_rxrb);
}

