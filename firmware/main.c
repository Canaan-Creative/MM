/*
 * Author: Minux
 * Author: Xiangfu Liu <xiangfu@openmobilefree.net>
 * Bitcoin:	1CanaaniJzgps8EV6Sfmpb7T8RutpaeyFn
 *
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "minilibc.h"
#include "system_config.h"
#include "defines.h"
#include "io.h"
#include "intr.h"
#include "uart.h"
#include "miner.h"
#include "sha256.h"
#include "alink.h"
#include "twipwm.h"
#include "shifter.h"
#include "timer.h"
#include "protocol.h"
#include "crc.h"

#include "hexdump.c"

#define IDLE_TIME	5	/* Seconds */
#define IDLE_TEMP	90	/* Degree (C) */

static uint8_t g_pkg[AVA2_P_COUNT];
static uint8_t g_act[AVA2_P_COUNT];
static int g_module_id = 0;	/* Default ID is 0 */
static int g_new_stratum = 0;
static int g_local_work = 0;
static int g_hw_work = 0;

uint32_t g_clock_conf_count = 0;


static uint32_t g_nonce2_offset = 0;
static uint32_t g_nonce2_range = 0xffffffff;

#define RET_RINGBUFFER_SIZE_RX 16
#define RET_RINGBUFFER_MASK_RX (RET_RINGBUFFER_SIZE_RX-1)
static uint8_t ret_buf[RET_RINGBUFFER_SIZE_RX][AVA2_P_DATA_LEN];
static volatile unsigned int ret_produce = 0;
static volatile unsigned int ret_consume = 0;

void delay(unsigned int ms)
{
	unsigned int i;

	while (ms && ms--) {
		for (i = 0; i < CPU_FREQUENCY / 1000 / 5; i++)
			__asm__ __volatile__("nop");
	}
}

static void encode_pkg(uint8_t *p, int type, uint8_t *buf, unsigned int len)
{
	uint32_t tmp;
	uint16_t crc;
	uint8_t *data;

	memset(p, 0, AVA2_P_COUNT);

	p[0] = AVA2_H1;
	p[1] = AVA2_H2;

	p[2] = type;
	p[3] = 1;
	p[4] = 1;

	data = p + 5;
	memcpy(data + 28, &g_module_id, 4); /* Attach the module_id at end */

	switch(type) {
	case AVA2_P_ACKDETECT:
	case AVA2_P_NONCE:
	case AVA2_P_TEST_RET:
		memcpy(data, buf, len);
		break;
	case AVA2_P_STATUS:
		tmp = read_temp0() << 16 | read_temp1();
		memcpy(data + 0, &tmp, 4);

		tmp = read_fan0() << 16 | read_fan1();
		memcpy(data + 4, &tmp, 4);

		tmp = get_asic_freq();
		memcpy(data + 8, &tmp, 4);
		tmp = get_voltage();
		memcpy(data + 12, &tmp, 4);

		memcpy(data + 16, &g_local_work, 4);
		memcpy(data + 20, &g_hw_work, 4);

		tmp = read_power_good();
		memcpy(data + 24, &tmp, 4);
		break;
	default:
		break;
	}

	crc = crc16(data, AVA2_P_DATA_LEN);
	p[AVA2_P_COUNT - 2] = crc & 0x00ff;
	p[AVA2_P_COUNT - 1] = (crc & 0xff00) >> 8;
}

void send_pkg(int type, uint8_t *buf, unsigned int len)
{
	debug32("Send: %d\n", type);
	encode_pkg(g_act, type, buf, len);
	uart_nwrite((char *)g_act, AVA2_P_COUNT);
}

static void polling()
{
	uint8_t *data;

	if (ret_consume == ret_produce) {
		send_pkg(AVA2_P_STATUS, NULL, 0);

		g_local_work = 0;
		g_hw_work = 0;
		return;
	}

	data = ret_buf[ret_consume];
	ret_consume = (ret_consume + 1) & RET_RINGBUFFER_MASK_RX;
	send_pkg(AVA2_P_NONCE, data, AVA2_P_DATA_LEN - 4);
	return;
}

static int decode_pkg(uint8_t *p, struct mm_work *mw)
{
	unsigned int expected_crc;
	unsigned int actual_crc;
	int idx, cnt;
	uint32_t tmp;

	uint8_t *data = p + 5;

	idx = p[3];
	cnt = p[4];

	debug32("Decode: %d %d/%d\n", p[2], idx, cnt);

	expected_crc = (p[AVA2_P_COUNT - 1] & 0xff) |
		((p[AVA2_P_COUNT - 2] & 0xff) << 8);

	actual_crc = crc16(data, AVA2_P_DATA_LEN);
	if(expected_crc != actual_crc) {
		debug32("PKG: CRC failed (W %08x, R %08x)\n",
			expected_crc, actual_crc);
		return 1;
	}

	timer_set(0, IDLE_TIME);
	switch (p[2]) {
	case AVA2_P_DETECT:
		g_new_stratum = 0;
		g_local_work = 0;
		g_hw_work = 0;
		alink_flush_fifo();
#if defined(AVALON3_A3233_MACHINE) || defined(AVALON3_A3233_CARD)
		led(2);
#else
		led(0);
#endif
		break;
	case AVA2_P_STATIC:
		g_new_stratum = 0;
		alink_flush_fifo();
		memcpy(&mw->coinbase_len, data, 4);
		memcpy(&mw->nonce2_offset, data + 4, 4);
		memcpy(&mw->nonce2_size, data + 8, 4);
		memcpy(&mw->merkle_offset, data + 12, 4);
		memcpy(&mw->nmerkles, data + 16, 4);
		memcpy(&mw->diff, data + 20, 4);
		memcpy(&mw->pool_no, data + 24, 4);
		debug32("D: (%d):  %d, %d, %d, %d, %d, %d, %d\n",
			g_new_stratum,
			mw->coinbase_len,
			mw->nonce2_offset,
			mw->nonce2_size,
			mw->merkle_offset,
			mw->nmerkles,
			mw->diff,
			mw->pool_no);
		break;
	case AVA2_P_JOB_ID:
		memcpy(mw->job_id, data, 4);
		hexdump(mw->job_id, 4);
		break;
	case AVA2_P_COINBASE:
		if (idx == 1)
			memset(mw->coinbase, 0, sizeof(mw->coinbase));
		memcpy(mw->coinbase + (idx - 1) * AVA2_P_DATA_LEN, data, AVA2_P_DATA_LEN);
		break;
	case AVA2_P_MERKLES:
		memcpy(mw->merkles[idx - 1], data, AVA2_P_DATA_LEN);
		break;
	case AVA2_P_HEADER:
		memcpy(mw->header + (idx - 1) * AVA2_P_DATA_LEN, data, AVA2_P_DATA_LEN);
		break;
	case AVA2_P_POLLING:
		memcpy(&tmp, data + 28, 4);
		debug32("ID: %d-%d\n", g_module_id, tmp);
		if (g_module_id != tmp)
			break;

		polling();

		memcpy(&tmp, data + 24, 4);
		if (tmp) {
			memcpy(&tmp, data, 4);
			adjust_fan(tmp);
			memcpy(&tmp, data + 4, 4);
			set_voltage(tmp);
#if defined(AVALON3_A3233_MACHINE) || defined(AVALON3_A3233_CARD)
			clko_init(1);

			/* ASIC Reset */
			led(2);
			delay(100);
			led(0);
			delay(100);
			led(2);
			delay(100);
#endif
			memcpy(&tmp, data + 8, 4);
			set_asic_freq(tmp);
			g_clock_conf_count = 0;
		}

		memcpy(&tmp, data + 12, 4);
#if defined(AVALON3_A3233_MACHINE) || defined(AVALON3_A3233_CARD)
		tmp |= 1 << 1;
#endif
		led(tmp);
		break;
	case AVA2_P_REQUIRE:
		break;
	case AVA2_P_SET:
		if (read_temp0() >= IDLE_TEMP || read_temp1() >= IDLE_TEMP)
			break;

		memcpy(&tmp, data, 4);
		adjust_fan(tmp);
		memcpy(&tmp, data + 4, 4);
		set_voltage(tmp);
#if defined(AVALON3_A3233_MACHINE) || defined(AVALON3_A3233_CARD)
		clko_init(1);

		/* ASIC Reset */
		led(2);
		delay(100);
		led(0);
		delay(100);
		led(2);
		delay(100);
#endif

		memcpy(&tmp, data + 8, 4);
		set_asic_freq(tmp);
		g_clock_conf_count = 0;

		memcpy(&g_nonce2_offset, data + 12, 4);
		memcpy(&g_nonce2_range, data + 16, 4);

		mw->nonce2 = g_nonce2_offset + (g_nonce2_range / AVA2_DEFAULT_MODULES) * g_module_id;
		alink_flush_fifo();

		g_new_stratum = 1;
		break;
	case AVA2_P_TARGET:
		memcpy(mw->target, data, AVA2_P_DATA_LEN);
		break;
	case AVA2_P_TEST:
		memcpy(&tmp, data + 28, 4);
		if (g_module_id == tmp) {
			set_voltage(ASIC_CORETEST_VOLT);
			led(1);
#if defined(AVALON3_A3233_MACHINE) || defined(AVALON3_A3233_CARD)
			clko_init(1);

			/* ASIC Reset */
			led(3);
			delay(100);
			led(1);
			delay(100);
			led(3);
			delay(100);
#endif
			alink_asic_test(0, ASIC_CORE_COUNT, 1);	/* Test all ASIC cores */
			led(0);
			set_voltage(ASIC_0V);
		}
		break;
	default:
		break;
	}

	return 0;
}

static int read_result(struct mm_work *mw, struct result *ret)
{
	uint8_t *data;
	int nonce;

	if (alink_rxbuf_empty())
		return 0;

#ifdef DEBUG
	alink_buf_status();
#endif

	alink_read_result(ret);
	g_local_work++;

	nonce = test_nonce(mw, ret);
	if (nonce == NONCE_HW) {
		g_hw_work++;
		return 1;
	}

	if (nonce == NONCE_DIFF) {
		data = ret_buf[ret_produce];
		ret_produce = (ret_produce + 1) & RET_RINGBUFFER_MASK_RX;
#if defined(AVALON3_A3233_MACHINE) || defined(AVALON3_A3233_CARD)
		uint32_t tmp;
		memcpy(&tmp, ret->nonce, 4);
		tmp = tmp - 0x1000 + 0x180;
		memcpy(ret->nonce, &tmp, 4);
#endif
		memcpy(data, (uint8_t *)ret, 20);
		memcpy(data + 20, mw->job_id, 4); /* Attach the job_id */
	}

	return 1;
}

static int get_pkg(struct mm_work *mw)
{
	static char pre_last, last;
	static int start = 0, count = 2;
	int tmp;

	while (1) {
		if (!uart_read_nonblock() && !start)
			break;

		pre_last = last;
		last = uart_read();

		if (start)
			g_pkg[count++] = last;

		if (count == AVA2_P_COUNT) {
			pre_last = last = 0;

			start = 0;
			count = 2;

			if (decode_pkg(g_pkg, mw)) {
#ifdef CFG_ENABLE_ACK
				send_pkg(AVA2_P_NAK, NULL, 0);
#endif
				return 1;
			} else {
				/* Here we send back PKG if necessary */
#ifdef CFG_ENABLE_ACK
				send_pkg(AVA2_P_ACK, NULL, 0);
#endif
				switch (g_pkg[2]) {
				case AVA2_P_DETECT:
					memcpy(&tmp, g_pkg + 5 + 28, 4);
					if (g_module_id == tmp)
						send_pkg(AVA2_P_ACKDETECT, (uint8_t *)MM_VERSION, MM_VERSION_LEN);
					break;
				case AVA2_P_REQUIRE:
					memcpy(&tmp, g_pkg + 5 + 28, 4);
					if (g_module_id == tmp)
						send_pkg(AVA2_P_STATUS, NULL, 0);
					break;
				default:
					break;
				}
			}
		}

		if (pre_last == AVA2_H1 && last == AVA2_H2 && !start) {
			g_pkg[0] = pre_last;
			g_pkg[1] = last;
			start = 1;
			count = 2;
		}
	}

	return 0;
}

void i2c_wr(){
	int i;
	for(i = 0; i < 10; i++){
		writel(i,0x80000708);
	}
}

void api_wr(unsigned int nonce2){
	unsigned int buf[23];
	unsigned int i;

        buf[ 0] = 0x1bed3ba0;
        buf[ 1] = 0xa2cb45c1;
        buf[ 2] = 0xd8f8ef67;
        buf[ 3] = 0x12146495;
        buf[ 4] = 0xc44192c0;
        buf[ 5] = 0x7145fd6d;
        buf[ 6] = 0x974bf4bb;
        buf[ 7] = 0x8f41371d;
        buf[ 8] = 0x65c90d1e;
        buf[ 9] = 0x9cb18a17;
        buf[10] = 0xfa77fe7d;
        buf[11] = 0x13cdfd7b;
        buf[12] = 0x00639107;
        buf[13] = 0x62a5f25c;
        buf[14] = 0x06b168ae;
        buf[15] = 0x087e051a;
        buf[16] = 0x89517050;
        buf[17] = 0x4ac1d001;
        buf[18] = nonce2 + 1;
        buf[19] = nonce2    ;
        buf[20] = 0x00000001;
        buf[21] = 0x00000001;
        buf[22] = 0x00000001;

	for(i = 0; i < 23; i++){
		writel(buf[i], 0x80000500);
	}
}

void api_rd(unsigned int * buf){
	int i;
	for(i = 0; i < 4; i++)
		buf[i] = readl(0x80000504);
}


void api_gen_hash_work(unsigned int i, unsigned int nonce2_h, unsigned int nonce2_l, unsigned int chip_num, unsigned int target_num, unsigned int * data){
	if(i%16 == 0 ){data[0]=0x1bed3ba0; data[1]=0xa2cb45c1; data[2]=0xd8f8ef67; data[3]=0x12146495; data[4]=0xc44192c0; data[5]=0x7145fd6d; data[6]=0x974bf4bb; data[7]=0x8f41371d; data[8]=0x65c90d1e; data[9]=0x9cb18a17; data[10]=0xfa77fe7d; data[11]=0x13cdfd7b; data[12]=0x00639107; data[13]=0x62a5f25c; data[14]=0x06b168ae; data[15]=0x087e051a; data[16]=0x89517050; data[17]=0x4ac1d001;}
	if(i%16 == 1 ){data[0]=0xc1680161; data[1]=0x9d8d4242; data[2]=0xe06e5fab; data[3]=0x25a54bbe; data[4]=0x222e8b87; data[5]=0x7848c34b; data[6]=0xeea79cd6; data[7]=0x528caf7e; data[8]=0x33cde02a; data[9]=0x983dab15; data[10]=0x8119ce2a; data[11]=0x1c9fc4ed; data[12]=0xdac8ce29; data[13]=0x6d0fd9da; data[14]=0x6e18f645; data[15]=0x087e051a; data[16]=0x2d547050; data[17]=0xe8dc86b1;}
	if(i%16 == 2 ){data[0]=0x84a9e522; data[1]=0xb6327d35; data[2]=0x2a54a40c; data[3]=0x879a9334; data[4]=0xd962d729; data[5]=0x5600f2af; data[6]=0xf81e2a4b; data[7]=0x80a27591; data[8]=0x4064bca0; data[9]=0x612fc425; data[10]=0xa1d03421; data[11]=0xf8941ad5; data[12]=0xc5e7acf1; data[13]=0xb0496c6a; data[14]=0xc35604f4; data[15]=0x087e051a; data[16]=0x3c537050; data[17]=0x826cce7a;}
	if(i%16 == 3 ){data[0]=0x1c11ef93; data[1]=0xb3baed4e; data[2]=0x2a54a40c; data[3]=0x879a9334; data[4]=0xd962d729; data[5]=0x5600f2af; data[6]=0xf81e2a4b; data[7]=0x80a27591; data[8]=0x4064bca0; data[9]=0x612fc425; data[10]=0xa1d03421; data[11]=0xfa941ad5; data[12]=0xc3efecf2; data[13]=0xb0496c6a; data[14]=0xc55604f4; data[15]=0x087e051a; data[16]=0x3e537050; data[17]=0x826cce7a;}
	if(i%16 == 4 ){data[0]=0xad258354; data[1]=0xcb517d22; data[2]=0x81c9ffff; data[3]=0xdd3da630; data[4]=0xea65fc9d; data[5]=0x857ccb58; data[6]=0x8a776518; data[7]=0x67eb9370; data[8]=0xed14a977; data[9]=0xf1d2de33; data[10]=0xa1235784; data[11]=0x65d344fe; data[12]=0xc6e2feca; data[13]=0x431284bc; data[14]=0x5e2e867c; data[15]=0x087e051a; data[16]=0x70527050; data[17]=0xc290e5a7;}
	if(i%16 == 5 ){data[0]=0xdaa34ad5; data[1]=0xb171b59c; data[2]=0xd8f8ef67; data[3]=0x12146495; data[4]=0xc44192c0; data[5]=0x7145fd6d; data[6]=0x974bf4bb; data[7]=0x8f41371d; data[8]=0x65c90d1e; data[9]=0x9cb18a17; data[10]=0xfa77fe7d; data[11]=0x19cdfd7b; data[12]=0x068a510a; data[13]=0x62a5f25c; data[14]=0x0cb168ae; data[15]=0x087e051a; data[16]=0x8f517050; data[17]=0x4ac1d001;}
	if(i%16 == 6 ){data[0]=0x010dceb6; data[1]=0x220f1dbd; data[2]=0xd8f8ef67; data[3]=0x12146495; data[4]=0xc44192c0; data[5]=0x7145fd6d; data[6]=0x974bf4bb; data[7]=0x8f41371d; data[8]=0x65c90d1e; data[9]=0x9cb18a17; data[10]=0xfa77fe7d; data[11]=0x12cdfd7b; data[12]=0x81677107; data[13]=0x62a5f25c; data[14]=0x05b168ae; data[15]=0x087e051a; data[16]=0x88517050; data[17]=0x4ac1d001;}
	if(i%16 == 7 ){data[0]=0x2bc36997; data[1]=0xb1231d47; data[2]=0x2a54a40c; data[3]=0x879a9334; data[4]=0xd962d729; data[5]=0x5600f2af; data[6]=0xf81e2a4b; data[7]=0x80a27591; data[8]=0x4064bca0; data[9]=0x612fc425; data[10]=0xa1d03421; data[11]=0xfc941ad5; data[12]=0xc1d82cf3; data[13]=0xb0496c6a; data[14]=0xc75604f4; data[15]=0x087e051a; data[16]=0x40537050; data[17]=0x826cce7a;}
	if(i%16 == 8 ){data[0]=0x62af5f08; data[1]=0x312bf034; data[2]=0x740d56be; data[3]=0xe2e8080f; data[4]=0x19fae6f7; data[5]=0xc35ad8db; data[6]=0x42727a8e; data[7]=0xff62389c; data[8]=0xc572d901; data[9]=0xcc39201c; data[10]=0x92f7791a; data[11]=0x24d6e2cd; data[12]=0x67d1e0c0; data[13]=0xf55f91a6; data[14]=0x39171075; data[15]=0x087e051a; data[16]=0x13557050; data[17]=0x9ed94986;}
	if(i%16 == 9 ){data[0]=0x2068a649; data[1]=0xb0cc4d3c; data[2]=0x2a54a40c; data[3]=0x879a9334; data[4]=0xd962d729; data[5]=0x5600f2af; data[6]=0xf81e2a4b; data[7]=0x80a27591; data[8]=0x4064bca0; data[9]=0x612fc425; data[10]=0xa1d03421; data[11]=0xf6941ad5; data[12]=0xc0016cf0; data[13]=0xb0496c6a; data[14]=0xc15604f4; data[15]=0x087e051a; data[16]=0x3a537050; data[17]=0x826cce7a;}
	if(i%16 == 10){data[0]=0xc95c48ca; data[1]=0x26fe7d8b; data[2]=0xd8f8ef67; data[3]=0x12146495; data[4]=0xc44192c0; data[5]=0x7145fd6d; data[6]=0x974bf4bb; data[7]=0x8f41371d; data[8]=0x65c90d1e; data[9]=0x9cb18a17; data[10]=0xfa77fe7d; data[11]=0x16cdfd7b; data[12]=0x8156f105; data[13]=0x62a5f25c; data[14]=0x09b168ae; data[15]=0x087e051a; data[16]=0x8c517050; data[17]=0x4ac1d001;}
	if(i%16 == 11){data[0]=0x1033995b; data[1]=0x9a7ce254; data[2]=0xe06e5fab; data[3]=0x25a54bbe; data[4]=0x222e8b87; data[5]=0x7848c34b; data[6]=0xeea79cd6; data[7]=0x528caf7e; data[8]=0x33cde02a; data[9]=0x983dab15; data[10]=0x8119ce2a; data[11]=0x189fc4ed; data[12]=0xdab84e2b; data[13]=0x6d0fd9da; data[14]=0x6a18f645; data[15]=0x087e051a; data[16]=0x29547050; data[17]=0xe8dc86b1;}
	if(i%16 == 12){data[0]=0x93753bfc; data[1]=0x2a6249eb; data[2]=0xe06e5fab; data[3]=0x25a54bbe; data[4]=0x222e8b87; data[5]=0x7848c34b; data[6]=0xeea79cd6; data[7]=0x528caf7e; data[8]=0x33cde02a; data[9]=0x983dab15; data[10]=0x8119ce2a; data[11]=0x239fc4ed; data[12]=0x545dae3e; data[13]=0x6d0fd9da; data[14]=0x7518f645; data[15]=0x087e051a; data[16]=0x34547050; data[17]=0xe8dc86b1;}
	if(i%16 == 13){data[0]=0x9ea1e81d; data[1]=0xa82e81f6; data[2]=0xe06e5fab; data[3]=0x25a54bbe; data[4]=0x222e8b87; data[5]=0x7848c34b; data[6]=0xeea79cd6; data[7]=0x528caf7e; data[8]=0x33cde02a; data[9]=0x983dab15; data[10]=0x8119ce2a; data[11]=0x249fc4ed; data[12]=0xd369ce3d; data[13]=0x6d0fd9da; data[14]=0x7618f645; data[15]=0x087e051a; data[16]=0x35547050; data[17]=0xe8dc86b1;}
	if(i%16 == 14){data[0]=0x873c2a5e; data[1]=0x2d1a8543; data[2]=0x81c9ffff; data[3]=0xdd3da630; data[4]=0xea65fc9d; data[5]=0x857ccb58; data[6]=0x8a776518; data[7]=0x67eb9370; data[8]=0xed14a977; data[9]=0xf1d2de33; data[10]=0xa1235784; data[11]=0x5ad344fe; data[12]=0x29ebded7; data[13]=0x431284bc; data[14]=0x532e867c; data[15]=0x087e051a; data[16]=0x65527050; data[17]=0xc290e5a7;}
	if(i%16 == 15){data[0]=0xa2bef63f; data[1]=0x30101538; data[2]=0x2a54a40c; data[3]=0x879a9334; data[4]=0xd962d729; data[5]=0x5600f2af; data[6]=0xf81e2a4b; data[7]=0x80a27591; data[8]=0x4064bca0; data[9]=0x612fc425; data[10]=0xa1d03421; data[11]=0xf7941ad5; data[12]=0x41054cf0; data[13]=0xb0496c6a; data[14]=0xc25604f4; data[15]=0x087e051a; data[16]=0x3b537050; data[17]=0x826cce7a;}

	data[0] = data[0] ^ (i & 0xfffffff0);

	data[2] = data[2] - (chip_num - target_num - 1);

	data[18] = nonce2_h;
	data[19] = nonce2_l;

	data[20] = 0x1;
	data[21] = 0x1;
	data[22] = 0x1;
}

void api_timeout(unsigned int timeout){
	writel(timeout, 0x8000050c);//timeout
}

void api_structure(unsigned int ch_num, unsigned int chip_num){
	unsigned int tmp;
	tmp = readl(0x80000510) & 0xff;//for sck timing reg
	tmp = tmp | (ch_num<<16) | (chip_num<<24);
	writel(tmp, 0x80000510);
}

void api_sck(unsigned int sck_div2){
	unsigned int tmp;
	tmp = (readl(0x80000510) & 0xffff0000) | sck_div2;
	writel(tmp, 0x80000510);
}

void api_write(unsigned int * data){
	unsigned int i;
	for(i = 0; i < 23; i++){
		writel(data[i], 0x80000500);
	}
}

void api_read(unsigned int * buf, unsigned int total_word){
	unsigned int i;
	for(i = 0; i < total_word; i++)
		buf[i] = readl(0x80000504);
}

void api_wait_rx(unsigned int total_word){
	unsigned int tmp = 0;
	while(tmp < total_word){
		tmp = readl(0x80000508);
		tmp = (tmp >> 20) & 0x1ff;
	}
}

void chip_rst(){
	led(8); delay(1000); led(0); delay(1000); led(8);
}

unsigned int iic_tx_cnt(){
	unsigned int tmp;
	tmp = readl(0x80000700);
	tmp = (tmp >> 9) & 0x1ff;
	return tmp;
}

unsigned int iic_rx_cnt(){
	unsigned int tmp;
	tmp = readl(0x80000700);
	tmp = tmp & 0x1ff;
	return tmp;
}

void iic_rst_logic(){
	unsigned int tmp = 0x1 << 23;
	writel(tmp, 0x80000700);
}

void iic_rst_rx(){
	unsigned int tmp = 0x1 << 21;
	writel(tmp, 0x80000700);
}

void iic_rst_tx(){
	unsigned int tmp = 0x1 << 22;
	writel(tmp, 0x80000700);
}

void iic_addr_set(unsigned int addr){
	writel(addr, 0x80000704);
}

unsigned int iic_addr_get(){
	return readl(0x80000704);
}

void iic_wait_wr_done(){
	unsigned int tmp;
	while(1){
		tmp = readl(0x80000700);
		tmp = tmp & (1<<18);
		if(tmp)
			break;
	}
	writel(tmp, 0x80000700);
}

unsigned int iic_wait_rd_done(){
	unsigned int tmp;
	while(1){
		tmp = readl(0x80000700);
		tmp = tmp & (3<<19);
		if(tmp)
			break;
	}
	
	writel(tmp, 0x80000700);
	tmp = tmp >> 19;
	return tmp;//>=2 means error
}

void iic_wr(unsigned int *data, unsigned int len){
	unsigned int i;
	for(i = 0; i < len; i++)
		writel(data[i], 0x80000708);
}

void iic_rd(unsigned int *data, unsigned int len){
	unsigned int i;
	for(i = 0; i < len; i++)
		data[i] = readl(0x8000070c);
}

void iic_loop(){
	unsigned int tmp = 0;
	while(1){
		tmp = iic_rx_cnt();
		if(tmp){
			tmp = readl(0x8000070c);
			writel(tmp, 0x80000708);
		}
	}
}

/*
DNA_PORT dna_port(
.DOUT  (dna_dout  ),
.CLK   (reg_dna[0]),
.DIN   (reg_dna[1]),
.READ  (reg_dna[2]),
.SHIFT (reg_dna[3])
*/

void dna_rd(unsigned int *data){
	unsigned int i, tmp;
	data[0] = 0;
	data[1] = 0;

	writel(0, 0x80000710); //idle

	writel(0|4, 0x80000710);
	writel(1|4, 0x80000710);
	tmp = readl(0x80000710);
	data[1] = (data[1]<<1) | ((tmp >> 4)&1);
	writel(0|4, 0x80000710);//shift

	writel(0, 0x80000710); //idle
	for(i = 1; i < 32; i++){
		writel(0|8, 0x80000710);
        	writel(1|8, 0x80000710);
		tmp = readl(0x80000710);
		data[1] = (data[1]<<1) | ((tmp >> 4)&1);
        	writel(0|8, 0x80000710); 
	}
	writel(0, 0x80000710); //idle

	for(; i < 57; i++){
		writel(0|8, 0x80000710);
        	writel(1|8, 0x80000710);
		data[0] = (data[1]>>31) | (data[0] << 1);
		data[1] = (data[1]<< 1) | ((readl(0x80000710) >> 4)&1);
        	writel(0|8, 0x80000710);
	}
}

void iic_reboot(){
        unsigned int tmp = 0x1 << 26;
        writel(tmp, 0x80000700);
}

/* func: chip_test
 ______    
|      |-----ch0------> (chip_num-1) +...+ chip1 + chip0
|  MM  |-----ch1------> (chip_num-1) +...+ chip1 + chip0
|______|-----ch_num-1-> (chip_num-1) +...+ chip1 + chip0
*/
unsigned char chip_test(
	unsigned int ch_num      ,//per MM, [10:1]
	unsigned int chip_num    ,//per channel, [5:1]
	unsigned int cal_core_num,//per chip, [248*16:1]
	unsigned int rx_word_num  //[5:4]
){
	unsigned int timeout  = 0x5000;
	unsigned int target_num = 0;//chip_num -1 .. 0
	unsigned int sck_div2 = 8;
	unsigned int total_word = (ch_num * chip_num) * rx_word_num;
	unsigned int data[23], i, j, k;
	unsigned int rxbuf[256];
	unsigned int nonce_respect, pass_cal_core_num = 0;

	//API CFG
	chip_rst();
	api_timeout(timeout);
	api_structure(ch_num, chip_num);
	api_sck(sck_div2);
	
	//test chip
	for(i = 0; i < chip_num; i++){
		target_num = i;
		for(j = 0; j < cal_core_num + 2; j++){
			//sent work
			api_gen_hash_work(j, j, j, chip_num, target_num, data);
			api_write(data);
			api_wait_rx(total_word);
			api_read(rxbuf, total_word);
			
			//verify nonce & nonce2
			if(j >= 2){
				api_gen_hash_work(j-2, j-2, j-2, chip_num, chip_num-1, data);
				nonce_respect = data[0] + 0x18000;
				for(k = 0; k < ch_num; k++){
					if(nonce_respect == rxbuf[k*rx_word_num + 2])
						pass_cal_core_num++;
				}
			}
		}
	}
	return pass_cal_core_num;
}

void led_set(unsigned int led){
	unsigned int tmp;
	tmp = (led<<4) | (readl(0x80000624) & 0xffffff0f);
	writel(tmp, 0x80000624);
}

void led_shifter(unsigned int led){
	unsigned int i, tmp;
	tmp = readl(0x80000624);
	for(i = 0; i < 8 ; i++){
		writel((tmp & 0xfffffffc) | (led & 1)    , 0x80000624);
		writel((tmp & 0xfffffffc) | (led & 1) | 2, 0x80000624);
		writel((tmp & 0xfffffffc) | (led & 1)    , 0x80000624);
		led = led >> 1;
	}
}

int main(int argv, char **argc)
{
	struct mm_work mm_work;
	struct work work;
	struct result result;

	//uart_init();
	//iic_reboot();

	//chip_test(10, 5, 248*16, 4);

	unsigned int i = 0;
	while(1){
		led_shifter(i);
		i++;
		led_set(0x5);
		delay(150);
		led_set(0xa);
		delay(150);
	}

	adjust_fan(0x1ff);/* Set the fan to 50% */
	alink_flush_fifo();

	wdg_init(1);
	wdg_feed((CPU_FREQUENCY / 1000) * 2); /* Configure the wdg to ~2 second, or it will reset FPGA */

	irq_setmask(0);
	irq_enable(1);

	g_module_id = read_module_id();

	uart_init();
	debug32("%d:MM-%s\n", g_module_id, MM_VERSION);
	debug32("T:%d, %d\n", read_temp0(), read_temp1());

	timer_set(0, IDLE_TIME);
	g_new_stratum = 0;

	/* Test part of ASIC cores */
	set_voltage(ASIC_CORETEST_VOLT);
	led(1);
#if defined(AVALON3_A3233_MACHINE) || defined(AVALON3_A3233_CARD)
	clko_init(1);

	/* ASIC Reset */
	led(3);
	delay(100);
	led(1);
	delay(100);
	led(3);
	delay(100);
#endif
	alink_asic_test(0, 2, 0);

	alink_asic_idle();
	set_voltage(ASIC_0V);

	while (1) {
		get_pkg(&mm_work);

		wdg_feed((CPU_FREQUENCY / 1000) * 2);
		if ((!timer_read(0) && g_new_stratum) ||
		    (read_temp0() >= IDLE_TEMP && read_temp1() >= IDLE_TEMP)) {
			g_new_stratum = 0;
			g_local_work = 0;
			g_hw_work = 0;
			alink_asic_idle();
			adjust_fan(0x1ff);
			set_voltage(ASIC_0V);
		}

		if (!g_new_stratum)
			continue;

		if (alink_txbuf_count() < (24 * 5)) {
			if (g_clock_conf_count < 100)
				g_clock_conf_count++;

			miner_gen_nonce2_work(&mm_work, mm_work.nonce2, &work);
			get_pkg(&mm_work);
			if (!g_new_stratum)
				continue;

			mm_work.nonce2++;
			miner_init_work(&mm_work, &work);
			alink_send_work(&work);
		}

		while (read_result(&mm_work, &result)) {
			get_pkg(&mm_work);
			if (!g_new_stratum)
				break;
		}
	}

	return 0;
}
