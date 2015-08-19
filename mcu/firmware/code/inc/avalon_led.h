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
#define LED_RED	0
#define LED_GREEN	1
#define LED_BLUE	2
#define LED_MCU	3

/* state defines */
#define LED_OFF	0
#define LED_ON	1
#define LED_BLINK	2
#define LED_BREATH	3

void led_init(void);
void led_set(unsigned int led, uint8_t led_op);
void led_rgb(unsigned int rgb);

#endif /* __AVALON_LED_H_ */
