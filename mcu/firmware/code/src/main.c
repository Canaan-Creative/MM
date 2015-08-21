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
static uint32_t g_ledstatus;
static uint16_t g_adc_val[ADC_CAPCOUNT];

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
	int ret, i;
	uint32_t tmp;
	uint8_t led_op, r, g, b;

	/* crc order is different with send */
	expected_crc = (pkg->crc[0] & 0xff)
			| ((pkg->crc[1] & 0xff) << 8);
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
		memset(g_ackpkg, 0, AVAM_P_COUNT);

		for (i = 0; i < ADC_CAPCOUNT; i++) {
			g_ackpkg[AVAM_P_DATAOFFSET + i * 2] = g_adc_val[i] >> 8;
			g_ackpkg[AVAM_P_DATAOFFSET + i * 2 + 1] = g_adc_val[i] & 0xff;
		}

		tmp = be32toh(g_ledstatus);
		memcpy(&g_ackpkg[AVAM_P_DATAOFFSET + ADC_CAPCOUNT * 2], &tmp, 4);

		init_mm_pkg((struct avalon_pkg *)g_ackpkg, AVAM_P_STATUS_M);
		uart_write(g_ackpkg, AVAM_P_COUNT);
		break;
	case AVAM_P_SETM:
		memcpy(&tmp, pkg->data + 4, 4);
		tmp = be32toh(tmp);
		r = (g_ledstatus >> 16) & 0xff;
		g = (g_ledstatus >> 8) & 0xff;
		b = g_ledstatus & 0xff;

		/* 2bits per color,  0: ignore, 1: rgb settings, 2: blink, 3: breath
		 * order: reserverd (2bit), r (2bit), g (2bit), b (2bit)
		 * */
		led_op = (tmp >> (4 + 24)) & 3;
		switch (led_op) {
			case 1:
				r = (tmp >> 16) & 0xff;
				led_rgb((r << 16) | (g << 8) | b);
				break;
			case 2:
				led_set(LED_RED, LED_BLINK);
				break;
			case 3:
				led_set(LED_RED, LED_BREATH);
				break;
			default:
				break;
		}

		led_op = (tmp >> (2 + 24)) & 3;
		switch (led_op) {
			case 1:
				g = (tmp >> 8) & 0xff;
				led_rgb((r << 16) | (g << 8) | b);
				break;
			case 2:
				led_set(LED_GREEN, LED_BLINK);
				break;
			case 3:
				led_set(LED_GREEN, LED_BREATH);
				break;
			default:
				break;
		}

		led_op = (tmp >> 24) & 3;
		switch (led_op) {
			case 1:
				b = tmp & 0xff;
				led_rgb((r << 16) | (g << 8) | b);
				break;
			case 2:
				led_set(LED_BLUE, LED_BLINK);
				break;
			case 3:
				led_set(LED_BLUE, LED_BREATH);
				break;
			default:
				break;
		}
		g_ledstatus = (tmp & 0xff000000) | (r << 16) | (g << 8) | b;
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
	uart_init();

	/* adc 0 ~ 1023 */
	for (i = 0; i < ADC_CAPCOUNT; i++)
		g_adc_val[i] = 0x3ff;

	for (i = LED_RED; i <= LED_MCU; i++)
		led_set(i, LED_ON);

	delay(2000);

	for (i = LED_RED; i <= LED_MCU; i++)
		led_set(i, LED_OFF);

	delay(2000);
	g_ledstatus = 0x15000000;

	timer_set(TIMER_ID1, IDLE_TIME, NULL);
	timer_set(TIMER_ID2, ADC_CAPTIME, NULL);
	while (1) {
		switch (stat) {
		case STAT_WORK:
			len = uart_rxrb_cnt();
			if (len >= AVAM_P_COUNT) {
				memset(g_reqpkg, 0, AVAM_P_COUNT);
				uart_read(g_reqpkg, AVAM_P_COUNT);
				process_mm_pkg((struct avalon_pkg*)g_reqpkg);
			}

			if (timer_istimeout(TIMER_ID1)) {
				stat = STAT_IDLE;
				led_set(LED_MCU, LED_ON);
			}
		break;
		case STAT_IDLE:
			len = uart_rxrb_cnt();
			if (len >= AVAM_P_COUNT) {
				stat = STAT_WORK;
				led_set(LED_MCU, LED_OFF);
			}

			__WFI();
			break;
		default:
			stat = STAT_IDLE;
			break;
		}

		if (timer_istimeout(TIMER_ID2)) {
			adc_read(ADC_RNTCP1, &g_adc_val[0]);
			adc_read(ADC_RNTCP2, &g_adc_val[1]);
			adc_read(ADC_RNTCP3, &g_adc_val[2]);
			adc_read(ADC_RNTCP4, &g_adc_val[3]);
			adc_read(ADC_VCC12VIN, &g_adc_val[4]);
			adc_read(ADC_VCC3V3, &g_adc_val[5]);

			timer_set(TIMER_ID2, ADC_CAPTIME, NULL);
		}
	}
}

