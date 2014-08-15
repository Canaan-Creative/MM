/*
 * Author: Mikeqin <Fengling.Qin@gmail.com>
 *
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */

#ifndef __IIC_H__
#define __IIC_H__

void iic_init(void);

uint32_t iic_read_cnt(void);
uint32_t iic_read(uint8_t *data, uint16_t len);
uint32_t iic_write(uint8_t *data, uint16_t len);

uint8_t iic_addr_get(void);
void iic_addr_set(unsigned char addr);

void iic_logic_reset(void);

void iic_rx_reset(void);
void iic_tx_reset(void);

#endif /* __IIC_H__ */
