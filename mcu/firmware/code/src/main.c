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
#ifdef __CODE_RED
#include <NXP/crp.h>
#endif
#include "crc.h"
#include "protocol.h"
#include "defines.h"
#include "libfunctions.h"

#include "avalon_uart.h"
#include "avalon_adc.h"
#include "avalon_led.h"
#include "avalon_timer.h"

#ifdef __CODE_RED
__CRP unsigned int CRP_WORD = CRP_NO_ISP;
#endif

#define STAT_WORK	0
#define STAT_IDLE	1

static uint8_t g_reqpkg[AVAM_P_COUNT];
static uint8_t g_ackpkg[AVAM_P_COUNT];
static uint32_t g_ledstatus = LED_GREEN;

static int init_mm_pkg(struct avalon_pkg *pkg, uint8_t type)
{
	uint16_t crc;

	pkg->head[0] = AVAM_H1;
	pkg->head[1] = AVAM_H2;
	pkg->type = type;
	pkg->opt = 0;
	pkg->idx = 1;
	pkg->cnt = 1;

	crc = crc16(pkg->data, AVAM_P_DATA_LEN);
	pkg->crc[0] = (crc & 0xff00) >> 8;
	pkg->crc[1] = crc & 0x00ff;
	return 0;
}

static unsigned int process_mm_pkg(struct avalon_pkg *pkg)
{
	unsigned int expected_crc;
	unsigned int actual_crc;
	int ret;

	expected_crc = (pkg->crc[1] & 0xff)
			| ((pkg->crc[0] & 0xff) << 8);
	actual_crc = crc16(pkg->data, AVAM_P_DATA_LEN);

	if (expected_crc != actual_crc)
		return 1;

	timer_set(TIMER_ID1, IDLE_TIME, NULL);
	switch (pkg->type) {
	case AVAM_P_DETECT:
		memset(g_ackpkg, 0, AVAM_P_COUNT);
		memcpy(g_ackpkg + AVAM_P_DATAOFFSET + AVAM_MM_DNA_LEN, AVAM_VERSION, AVAM_MM_VER_LEN);
		init_mm_pkg((struct avalon_pkg *)g_ackpkg, AVAM_P_ACKDETECT);
		uart_write(g_ackpkg, AVAM_P_COUNT);
		ret = 0;
		break;
	case AVAM_P_POLLING:
		memset(g_ackpkg, 0xff, AVAM_P_COUNT);
		init_mm_pkg((struct avalon_pkg *)g_ackpkg, AVAM_P_STATUS_M);
		uart_write(g_ackpkg, AVAM_P_COUNT);
		break;
	default:
		ret = 1;
		break;
	}

	return ret;
}

int main(void)
{
	uint8_t stat = STAT_WORK;
	uint32_t len = 0;
	uint8_t i = 0;

	Board_Init();
	SystemCoreClockUpdate();

	timer_init();
	led_init();
	adc_init();

	for (i = LED_RED; i <= LED_MCU; i++)
		led_set(i, LED_ON);

	delay(2000);

	for (i = LED_RED; i <= LED_MCU; i++)
		led_set(i, LED_OFF);

	delay(2000);

	led_rgb(0xffffff);
	delay(2000);
	led_rgb(0x0);
	delay(2000);

	/* post */
	for (i = LED_RED; i <= LED_MCU; i++) {
		led_set(i, LED_BLINK);
		delay(5000);
		led_set(i, LED_BREATH);
		delay(5000);
		led_set(i, LED_OFF);
		delay(5000);
		led_set(i, LED_ON);
		delay(5000);
		led_set(i, LED_OFF);
	}

	timer_set(TIMER_ID1, IDLE_TIME, NULL);
	while (1) {
		switch (stat) {
		case STAT_WORK:
			len = uart_rxrb_cnt();
			if (len >= AVAM_P_COUNT) {
				memset(g_reqpkg, 0, AVAM_P_COUNT);
				uart_read(g_reqpkg, AVAM_P_COUNT);
				process_mm_pkg((struct avalon_pkg*)g_reqpkg);
			}

			if (timer_istimeout(TIMER_ID1))
				stat = STAT_IDLE;
		break;
		case STAT_IDLE:
			g_ledstatus = LED_GREEN;
			led_rgb(g_ledstatus);
			len = uart_rxrb_cnt();
			if (len >= AVAM_P_COUNT)
				stat = STAT_WORK;

			__WFI();
			break;
		default:
			stat = STAT_IDLE;
			break;
		}
	}
}

