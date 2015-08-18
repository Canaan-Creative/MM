/*
 * @brief led head file
 *
 * @note
 *
 * @par
 */
#ifndef __AVALON_LED_H_
#define __AVALON_LED_H_

/* RGB defines */
#define LED_GREEN	0xff00
#define LED_RED	0xff0000
#define LED_BLUE	0xff
#define LED_BLACK	0
#define LED_WHITE	0xffffff

/* state defines */
#define STATE_LED_OFF	0
#define STATE_LED_ON	1
#define STATE_LED_BLINK	2
#define STATE_LED_BREATH	3

void led_init(void);
void led_rgb(unsigned int rgb);
void led_blink(unsigned int rgb);
void led_mcu(uint8_t stat);

#endif /* __AVALON_LED_H_ */
