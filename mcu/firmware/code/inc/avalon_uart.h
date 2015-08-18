/*
 * @brief uart head file
 *
 * @note
 *
 * @par
 */
#ifndef __AVALON_UART_H_
#define __AVALON_UART_H_

void uart_init(void);
uint32_t uart_rxrb_cnt(void);
uint32_t uart_read(uint8_t *pbuf, uint32_t buf_len);
uint32_t uart_write(uint8_t *pbuf, uint32_t len);
void uart_flush_txrb(void);
void uart_flush_rxrb(void);

#endif /* __AVALON_UART_H_ */
