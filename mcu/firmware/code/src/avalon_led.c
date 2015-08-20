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

#define PIN_LED_RED	9
#define PIN_LED_GREEN	15
#define PIN_LED_BLUE	8
#define PIN_LED_MCU	16

#define DUTY_100	(uint32_t)(256)
#define DUTY_50		(uint32_t)(DUTY_100*0.5)
#define DUTY_25		(uint32_t)(DUTY_100*0.75)
#define DUTY_10		(uint32_t)(DUTY_100*0.9)
#define DUTY_0		(0)

#define PWM_INDEX_MAX	2

static volatile uint8_t mcu_breath;
static volatile uint8_t mcu_blink;
static volatile uint8_t led_breath[3];
static volatile uint8_t led_blink[3];

/*
 * http://sean.voisen.org/blog/2011/10/breathing-led-with-arduino
 * exp(sin(x * 2 / 3.14)) - 0.37) * 108.49
 */
static uint8_t led_pwmcalc(uint8_t index)
{
	static uint8_t pwmticks[PWM_INDEX_MAX];
	uint8_t pwm_dat[] = {
		0, 4, 29, 88, 183, 252, 219, 123, 47, 10
	};

	if (index > PWM_INDEX_MAX)
		return 0;

	return pwm_dat[pwmticks[index]++ % 10];
}

static void led_setduty(uint8_t r_duty, uint8_t g_duty, uint8_t b_duty)
{
	/* CT16B0_MAT0(B)/CT16B0_MAT1(R)/CT16B0_MAT2(G) */
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, PIN_LED_RED, IOCON_FUNC2 | IOCON_MODE_INACT);
	Chip_IOCON_PinMuxSet(LPC_IOCON, 1, PIN_LED_GREEN, IOCON_FUNC2 | IOCON_MODE_INACT);
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, PIN_LED_BLUE, IOCON_FUNC2 | IOCON_MODE_INACT);

	Chip_TIMER_ClearMatch(LPC_TIMER16_0, 1);
	Chip_TIMER_ExtMatchControlSet(LPC_TIMER16_0, 1, TIMER_EXTMATCH_SET, 1);
	Chip_TIMER_SetMatch(LPC_TIMER16_0, 1, r_duty);

	Chip_TIMER_ClearMatch(LPC_TIMER16_0, 2);
	Chip_TIMER_ExtMatchControlSet(LPC_TIMER16_0, 1, TIMER_EXTMATCH_SET, 2);
	Chip_TIMER_SetMatch(LPC_TIMER16_0, 2, g_duty);

	Chip_TIMER_ClearMatch(LPC_TIMER16_0, 0);
	Chip_TIMER_ExtMatchControlSet(LPC_TIMER16_0, 1, TIMER_EXTMATCH_SET, 0);
	Chip_TIMER_SetMatch(LPC_TIMER16_0, 0, b_duty);
}

static void led_blinkproc(void)
{
	static uint8_t open = 0;

	if (open) {
		if (led_blink[0]) {
			Chip_IOCON_PinMuxSet(LPC_IOCON, 0, PIN_LED_RED, IOCON_FUNC0);
			Chip_GPIO_SetPinState(LPC_GPIO, 0, PIN_LED_RED, false);
		}
		if (led_blink[1]) {
			Chip_IOCON_PinMuxSet(LPC_IOCON, 1, PIN_LED_GREEN, IOCON_FUNC0 | IOCON_MODE_INACT);
			Chip_GPIO_SetPinState(LPC_GPIO, 1, PIN_LED_GREEN, false);
		}
		if (led_blink[2]) {
			Chip_IOCON_PinMuxSet(LPC_IOCON, 0, PIN_LED_BLUE, IOCON_FUNC0 | IOCON_MODE_INACT);
			Chip_GPIO_SetPinState(LPC_GPIO, 0, PIN_LED_BLUE, false);
		}
	} else {
		if (led_blink[0]) {
			Chip_IOCON_PinMuxSet(LPC_IOCON, 0, PIN_LED_RED, IOCON_FUNC0);
			Chip_GPIO_SetPinState(LPC_GPIO, 0, PIN_LED_RED, true);
		}
		if (led_blink[1]) {
			Chip_IOCON_PinMuxSet(LPC_IOCON, 1, PIN_LED_GREEN, IOCON_FUNC0 | IOCON_MODE_INACT);
			Chip_GPIO_SetPinState(LPC_GPIO, 1, PIN_LED_GREEN, true);
		}
		if (led_blink[2]) {
			Chip_IOCON_PinMuxSet(LPC_IOCON, 0, PIN_LED_BLUE, IOCON_FUNC0 | IOCON_MODE_INACT);
			Chip_GPIO_SetPinState(LPC_GPIO, 0, PIN_LED_BLUE, true);
		}
	}
	open = ~open;
}

void TIMER16_0_IRQHandler(void)
{
        static uint32_t timer_cnt;

        Chip_TIMER_ClearMatch(LPC_TIMER16_0, 3);

	/* CT16B0_MAT0(B)/CT16B0_MAT1(R)/CT16B0_MAT2(G) */
        if ((led_breath[0] || led_breath[1] || led_breath[2]) && !timer_cnt) {
                uint8_t pwm = led_pwmcalc(0);

		/* LED R */
                if (led_breath[0]) {
                        Chip_TIMER_ClearMatch(LPC_TIMER16_0, 1);
                        Chip_TIMER_ExtMatchControlSet(LPC_TIMER16_0, 1, TIMER_EXTMATCH_SET, 1);
                        Chip_TIMER_SetMatch(LPC_TIMER16_0, 1, pwm);
                }
		/* LED G */
                if (led_breath[1]) {
                        Chip_TIMER_ClearMatch(LPC_TIMER16_0, 2);
                        Chip_TIMER_ExtMatchControlSet(LPC_TIMER16_0, 1, TIMER_EXTMATCH_SET, 2);
                        Chip_TIMER_SetMatch(LPC_TIMER16_0, 2, pwm);
                }
		/* LED B */
		if (led_breath[2]) {
                        Chip_TIMER_ClearMatch(LPC_TIMER16_0, 0);
                        Chip_TIMER_ExtMatchControlSet(LPC_TIMER16_0, 1, TIMER_EXTMATCH_SET, 0);
                        Chip_TIMER_SetMatch(LPC_TIMER16_0, 0, pwm);
                }
        }

        /* process blink */
        if ((led_blink[0] || led_blink[1] || led_blink[2]) && !timer_cnt)
		led_blinkproc();

        /* color per second = 40 / 188 (~0.21s) */
        if ((led_blink[0] || led_blink[1] || led_blink[2] ||
		led_breath[0] || led_breath[1] || led_breath[2])
		&& (timer_cnt++ == 40))
                timer_cnt = 0;
}

void led_init(void)
{
	/* System CLK 48MHz */
	Chip_TIMER_Init(LPC_TIMER16_0);
	Chip_TIMER_Disable(LPC_TIMER16_0);

	/* CT16B0_MAT0(B)/CT16B0_MAT1(R)/CT16B0_MAT2(G) init */
	Chip_IOCON_PinMuxSet(LPC_IOCON, 1, PIN_LED_GREEN, IOCON_FUNC0 | IOCON_MODE_INACT);
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, PIN_LED_BLUE, IOCON_FUNC0 | IOCON_MODE_INACT);
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, PIN_LED_RED, IOCON_FUNC0 | IOCON_MODE_INACT);

	Chip_GPIO_SetPinDIROutput(LPC_GPIO, 1, PIN_LED_GREEN);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, PIN_LED_BLUE);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, PIN_LED_RED);

	Chip_GPIO_SetPinState(LPC_GPIO, 1, PIN_LED_GREEN, true);
	Chip_GPIO_SetPinState(LPC_GPIO, 0, PIN_LED_BLUE, true);
	Chip_GPIO_SetPinState(LPC_GPIO, 0, PIN_LED_RED, true);

	Chip_TIMER_PrescaleSet(LPC_TIMER16_0, 999);
	Chip_TIMER_SetMatch(LPC_TIMER16_0, 3, DUTY_100);
	Chip_TIMER_MatchEnableInt(LPC_TIMER16_0, 3);
	Chip_TIMER_ResetOnMatchEnable(LPC_TIMER16_0, 3);
	/* CT16B0_MAT0/CT16B0_MAT1/CT16B0_MAT2 enable */
	LPC_TIMER16_0->PWMC = 0x7;
	NVIC_ClearPendingIRQ(TIMER_16_0_IRQn);
	NVIC_EnableIRQ(TIMER_16_0_IRQn);
	Chip_TIMER_Enable(LPC_TIMER16_0);

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

static void led_r(uint8_t led_op)
{
	led_breath[0] = 0;
	led_blink[0] = 0;
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, PIN_LED_RED, IOCON_FUNC0);

	if (led_op == LED_OFF)
		Chip_GPIO_SetPinState(LPC_GPIO, 0, PIN_LED_RED, true);

	if (led_op == LED_ON)
		Chip_GPIO_SetPinState(LPC_GPIO, 0, PIN_LED_RED, false);

	if (led_op == LED_BLINK)
		led_blink[0] = 1;

	if (led_op == LED_BREATH) {
		Chip_IOCON_PinMuxSet(LPC_IOCON, 0, PIN_LED_RED, IOCON_FUNC2 | IOCON_MODE_INACT);
		led_breath[0] = 1;
	}
}

static void led_g(uint8_t led_op)
{
	led_breath[1] = 0;
	led_blink[1] = 0;
	Chip_IOCON_PinMuxSet(LPC_IOCON, 1, PIN_LED_GREEN, IOCON_FUNC0 | IOCON_MODE_INACT);

	if (led_op == LED_OFF)
		Chip_GPIO_SetPinState(LPC_GPIO, 1, PIN_LED_GREEN, true);

	if (led_op == LED_ON)
		Chip_GPIO_SetPinState(LPC_GPIO, 1, PIN_LED_GREEN, false);

	if (led_op == LED_BLINK)
		led_blink[1] = 1;

	if (led_op == LED_BREATH) {
		Chip_IOCON_PinMuxSet(LPC_IOCON, 1, PIN_LED_GREEN, IOCON_FUNC2 | IOCON_MODE_INACT);
		led_breath[1] = 1;
	}
}

static void led_b(uint8_t led_op)
{
	led_breath[2] = 0;
	led_blink[2] = 0;
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, PIN_LED_BLUE, IOCON_FUNC0 | IOCON_MODE_INACT);

	if (led_op == LED_OFF)
		Chip_GPIO_SetPinState(LPC_GPIO, 0, PIN_LED_BLUE, true);

	if (led_op == LED_ON)
		Chip_GPIO_SetPinState(LPC_GPIO, 0, PIN_LED_BLUE, false);

	if (led_op == LED_BLINK)
		led_blink[2] = 1;

	if (led_op == LED_BREATH) {
		Chip_IOCON_PinMuxSet(LPC_IOCON, 0, PIN_LED_BLUE, IOCON_FUNC2 | IOCON_MODE_INACT);
		led_breath[2] = 1;
	}
}

void led_rgb(unsigned int rgb)
{
	uint8_t r, g, b;

	led_breath[0] = led_breath[1] = led_breath[2] = 0;
	led_blink[0] = led_blink[1] = led_blink[2] = 0;

	r = (rgb >> 16) & 0xff;
	g = (rgb >> 8) & 0xff;
	b = rgb & 0xff;

	led_setduty(r, g, b);
}

static void led_mcublinkproc(void)
{
	static uint8_t open = 0;

	if (open)
		Chip_GPIO_SetPinState(LPC_GPIO, 0, PIN_LED_MCU, true);
	else
		Chip_GPIO_SetPinState(LPC_GPIO, 0, PIN_LED_MCU, false);
	open = ~open;
}

void TIMER32_1_IRQHandler(void)
{
	static uint32_t timer_cnt;

	Chip_TIMER_ClearMatch(LPC_TIMER32_1, 0);

	if (mcu_breath && !timer_cnt) {
		uint8_t pwm = led_pwmcalc(1);

		if (mcu_breath) {
			Chip_TIMER_ClearMatch(LPC_TIMER32_1, 3);
			Chip_TIMER_ExtMatchControlSet(LPC_TIMER32_1, 1, TIMER_EXTMATCH_SET, 3);
			Chip_TIMER_SetMatch(LPC_TIMER32_1, 3, pwm);
		}
	}

	/* process blink */
	if (mcu_blink && !timer_cnt)
		led_mcublinkproc();

	/* color per second = 40 / 188 (~0.21s) */
	if ((mcu_blink || mcu_breath) && (timer_cnt++ == 40))
		timer_cnt = 0;
}

static void led_mcu(uint8_t led_op)
{
	mcu_breath = 0;
	mcu_blink = 0;
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, PIN_LED_MCU, IOCON_FUNC0 | IOCON_MODE_INACT);

	if (led_op == LED_OFF)
		Chip_GPIO_SetPinState(LPC_GPIO, 0, PIN_LED_MCU, false);

	if (led_op == LED_ON)
		Chip_GPIO_SetPinState(LPC_GPIO, 0, PIN_LED_MCU, true);

	if (led_op == LED_BLINK)
		mcu_blink = 1;

	if (led_op == LED_BREATH) {
		Chip_IOCON_PinMuxSet(LPC_IOCON, 0, PIN_LED_MCU, IOCON_FUNC2 | IOCON_MODE_INACT);
		mcu_breath = 1;
	}
}

void led_set(unsigned int led, uint8_t led_op)
{
	if (led == LED_MCU)
		led_mcu(led_op);

	if (led == LED_RED)
		led_r(led_op);

	if (led == LED_GREEN)
		led_g(led_op);

	if (led == LED_BLUE)
		led_b(led_op);
}

