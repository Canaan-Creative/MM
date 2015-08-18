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

#include "board.h"
#include "avalon_led.h"
#include "avalon_timer.h"

#define PIN_LED_RED	15
#define PIN_LED_GREEN	8
#define PIN_LED_BLUE	9
#define PIN_LED_MCU	16

#define DUTY_100	(uint32_t)(256)
#define DUTY_50		(uint32_t)(DUTY_100*0.5)
#define DUTY_25		(uint32_t)(DUTY_100*0.75)
#define DUTY_10		(uint32_t)(DUTY_100*0.9)
#define DUTY_0		(0)

#define TIMER_LED	TIMER_ID2
#define TIMER_LED_MCU	TIMER_ID3

static uint32_t blinkcolor;
static volatile uint8_t mcu_breath;

static void led_setduty(uint8_t r_duty, uint8_t g_duty, uint8_t b_duty)
{
	Chip_TIMER_ClearMatch(LPC_TIMER16_0, 2);
	Chip_TIMER_ExtMatchControlSet(LPC_TIMER16_0, 1, TIMER_EXTMATCH_SET, 2);
	Chip_TIMER_SetMatch(LPC_TIMER16_0, 2, r_duty);

	Chip_TIMER_ClearMatch(LPC_TIMER16_0, 0);
	Chip_TIMER_ExtMatchControlSet(LPC_TIMER16_0, 1, TIMER_EXTMATCH_SET, 0);
	Chip_TIMER_SetMatch(LPC_TIMER16_0, 0, g_duty);

	Chip_TIMER_ClearMatch(LPC_TIMER16_0, 1);
	Chip_TIMER_ExtMatchControlSet(LPC_TIMER16_0, 1, TIMER_EXTMATCH_SET, 1);
	Chip_TIMER_SetMatch(LPC_TIMER16_0, 1, b_duty);
}

static void led_set(uint32_t rgb)
{
	uint8_t r, g, b;

	r = (rgb >> 16) & 0xff;
	g = (rgb >> 8) & 0xff;
	b = (rgb & 0xff);

	/* FIXME:PWM set need delay some times? */
	Chip_TIMER_Disable(LPC_TIMER16_0);
	led_setduty(r, g, b);

	/* Prescale 0 */
	Chip_TIMER_PrescaleSet(LPC_TIMER16_0, 0);

	/* PWM Period 800Hz */
	Chip_TIMER_SetMatch(LPC_TIMER16_0, 3, DUTY_100);
	Chip_TIMER_ResetOnMatchEnable(LPC_TIMER16_0, 3);

	/* CT16B0_MAT0/CT16B0_MAT1/CT16B0_MAT2 enable */
	LPC_TIMER16_0->PWMC = 0x7;

	Chip_TIMER_Enable(LPC_TIMER16_0);
}

static void led_blinkcb(void)
{
	static uint8_t open = 0;

	if (open)
		led_set(LED_BLACK);
	else
		led_set(blinkcolor);
	open = ~open;
}

void led_init(void)
{
	/* System CLK 48MHz */
	Chip_TIMER_Init(LPC_TIMER16_0);
	Chip_TIMER_Disable(LPC_TIMER16_0);

	/* CT16B0_MAT0/CT16B0_MAT1/CT16B0_MAT2 init */
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, PIN_LED_GREEN, IOCON_FUNC2 | IOCON_MODE_INACT);
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, PIN_LED_BLUE, IOCON_FUNC2 | IOCON_MODE_INACT);
	Chip_IOCON_PinMuxSet(LPC_IOCON, 1, PIN_LED_RED, IOCON_FUNC2 | IOCON_MODE_INACT);

	/* CT16B0_MAT0 LED_GREEN duty:50% */
	Chip_TIMER_ExtMatchControlSet(LPC_TIMER16_0, 1, TIMER_EXTMATCH_SET, 0);
	Chip_TIMER_SetMatch(LPC_TIMER16_0, 0, DUTY_0);

	/* CT16B0_MAT1 LED_BLUE duty:25% */
	Chip_TIMER_ExtMatchControlSet(LPC_TIMER16_0, 1, TIMER_EXTMATCH_SET, 1);
	Chip_TIMER_SetMatch(LPC_TIMER16_0, 1, DUTY_0);

	/* CT16B0_MAT2 LED_RED duty:10% */
	Chip_TIMER_ExtMatchControlSet(LPC_TIMER16_0, 1, TIMER_EXTMATCH_SET, 2);
	Chip_TIMER_SetMatch(LPC_TIMER16_0, 2, DUTY_0);

	/* led mcu */
	Chip_TIMER_Init(LPC_TIMER32_1);
	Chip_TIMER_Disable(LPC_TIMER32_1);

	/* CT32B1_MAT3 init */
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, PIN_LED_MCU, IOCON_FUNC0 | IOCON_MODE_INACT);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, PIN_LED_MCU);
	Chip_GPIO_SetPinState(LPC_GPIO, 0, PIN_LED_MCU, false);

	Chip_TIMER_PrescaleSet(LPC_TIMER32_1, 999);
	Chip_TIMER_SetMatch(LPC_TIMER32_1, 0, DUTY_100);
	Chip_TIMER_MatchEnableInt(LPC_TIMER32_1, 0);
	Chip_TIMER_ResetOnMatchEnable(LPC_TIMER32_1, 0);
	LPC_TIMER32_1->PWMC = 1 << 3;

	NVIC_ClearPendingIRQ(TIMER_32_1_IRQn);
	NVIC_EnableIRQ(TIMER_32_1_IRQn);
	Chip_TIMER_Enable(LPC_TIMER32_1);
}

void led_rgb(unsigned int rgb)
{
	timer_kill(TIMER_LED);
	led_set(rgb);
}

void led_blink(unsigned int rgb)
{
	blinkcolor = rgb;
	timer_set(TIMER_LED, 500, led_blinkcb);
}

/*
 * http://sean.voisen.org/blog/2011/10/breathing-led-with-arduino
 * exp(sin(x * 2 / 3.14)) - 0.37) * 108.49
 */
static uint8_t led_pwmcalc(void)
{
	static uint8_t pwmticks;
	uint8_t pwm_dat[] = {
		0, 4, 29, 88, 183, 252, 219, 123, 47, 10
	};

	return pwm_dat[pwmticks++ % 10];
}

void TIMER32_1_IRQHandler(void)
{
	static uint32_t timer_cnt;

	Chip_TIMER_ClearMatch(LPC_TIMER16_0, 1);

	if (mcu_breath && !timer_cnt) {
		uint8_t pwm = led_pwmcalc();

		if (mcu_breath) {
			Chip_TIMER_ClearMatch(LPC_TIMER16_0, 3);
			Chip_TIMER_ExtMatchControlSet(LPC_TIMER16_0, 1, TIMER_EXTMATCH_SET, 3);
			Chip_TIMER_SetMatch(LPC_TIMER16_0, 3, pwm);
		}
	}

	/* color per second = 40 / 188 (~0.21s) */
	if (mcu_breath && (timer_cnt++ == 40))
		timer_cnt = 0;
}

static void led_mcublinkcb(void)
{
	static uint8_t open = 0;

	if (open)
		Chip_GPIO_SetPinState(LPC_GPIO, 0, PIN_LED_MCU, true);
	else
		Chip_GPIO_SetPinState(LPC_GPIO, 0, PIN_LED_MCU, false);
	open = ~open;
}

void led_mcu(uint8_t stat)
{
	mcu_breath = 0;
	timer_kill(TIMER_LED_MCU);
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, PIN_LED_MCU, IOCON_FUNC0 | IOCON_MODE_INACT);

	if (stat == STATE_LED_OFF)
		Chip_GPIO_SetPinState(LPC_GPIO, 0, PIN_LED_MCU, false);

	if (stat == STATE_LED_ON)
		Chip_GPIO_SetPinState(LPC_GPIO, 0, PIN_LED_MCU, true);

	if (stat == STATE_LED_BLINK)
		timer_set(TIMER_LED, 500, led_mcublinkcb);

	if (stat == STATE_LED_BLINK) {
		Chip_IOCON_PinMuxSet(LPC_IOCON, 0, PIN_LED_MCU, IOCON_FUNC2 | IOCON_MODE_INACT);
		mcu_breath = 1;
	}
}

