/*
 * Author: Mikeqin <Fengling.Qin@gmail.com>
 *
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */

#ifndef __I2C_H__
#define __I2C_H__

void iic_init(void);
int iic_read(uint8_t *data, uint16_t len);
uint16_t iic_read_cnt(void);
uint16_t iic_write(uint8_t *data, uint16_t len);
unsigned char iic_addr_get(void);
void iic_addr_set(unsigned char addr);
void iic_logic_reset(void);
void iic_rx_reset(void);
void iic_tx_reset(void);

#endif /* __I2C_H__ */
