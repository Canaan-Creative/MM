/*
 * Author: Xiangfu Liu <xiangfu@openmobilefree.net>
 * Bitcoin:	1CanaaniJzgps8EV6Sfmpb7T8RutpaeyFn
 *
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */

#ifndef _TWI_H_
#define _TWI_H_

void wdg_feed(uint32_t value);

void adjust_fan(uint32_t pwm);

uint32_t read_fan(void);
int16_t read_temp(void);
void lcd_init(void);
void lcd_on(void);
void lcd_off(void);
void lcd_clear(void);
void lcd_home(void);
void lcd_setcursor(uint8_t col, uint8_t row);
void lcd_leftscroll(void);
void lcd_rightscroll(void);
void lcd_write(char c);
void lcd_puts(const char *s);

#endif	/* _TWI_H_ */
