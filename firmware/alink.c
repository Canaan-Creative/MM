/*
 * Author: Xiangfu Liu <xiangfu@openmobilefree.net>
 * Bitcoin:	1CanaaniJzgps8EV6Sfmpb7T8RutpaeyFn
 *
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */

#include <stdint.h>

#include "minilibc.h"
#include "system_config.h"
#include "defines.h"
#include "io.h"
#include "uart.h"
#include "miner.h"

static struct lm32_alink *alink = (struct lm32_alink *)ALINK_BASE;

void alink_init(uint32_t count)
{
	/* Enalbe how many PHY in controller, base on the count of 1
	 * 01: 0x1
	 * 02: 0x3
	 * ...
	 * 08: 0xff
	 * 10: 0x3ff
	 */
	writel(count, &alink->en);
}

int alink_busy_status()
{
	return readl(&alink->busy);
}

int alink_txbuf_full()
{
	uint32_t value = LM32_ALINK_STATE_TXFULL & readl(&alink->state);
	debug32("TX: full %d\n", value);
	return value;
}

int alink_txbuf_count()
{
	uint32_t value;

	value = readl(&alink->state);
	return ((value & LM32_ALINK_STATE_TXCOUNT) >> 4);
}

int alink_rxbuf_count()
{
	uint32_t value;

	value = readl(&alink->state);
	return ((value & LM32_ALINK_STATE_RXCOUNT) >> 20);
}

int alink_rxbuf_empty()
{
	return (LM32_ALINK_STATE_RXEMPTY & readl(&alink->state));
}

int alink_send_work(struct work *w)
{
	uint32_t tmp;
	int i;

	/* The chip configure information */
	memcpy((uint8_t *)(&tmp), w->task_id, 4);
	writel(tmp, &alink->tx);

	memcpy((uint8_t *)(&tmp), w->task_id + 4, 4);
	writel(tmp, &alink->tx);

	memcpy((uint8_t *)(&tmp), w->step, 4);
	writel(tmp, &alink->tx);

	memcpy((uint8_t *)(&tmp), w->timeout, 4);
	writel(tmp, &alink->tx);

	memcpy((uint8_t *)(&tmp), w->clock, 4);
	writel(tmp, &alink->tx);

	memcpy((uint8_t *)(&tmp), w->clock + 4, 4);
	writel(tmp, &alink->tx);

	/* Task data */
	for (i = 8; i >= 0; i -= 4) {
		memcpy((uint8_t *)(&tmp), w->data + 32 + i, 4);
		writel(tmp, &alink->tx);
	}

	memcpy((uint8_t *)(&tmp), w->a1, 4);
	writel(tmp, &alink->tx);
	memcpy((uint8_t *)(&tmp), w->a0, 4);
	writel(tmp, &alink->tx);
	memcpy((uint8_t *)(&tmp), w->e2, 4);
	writel(tmp, &alink->tx);
	memcpy((uint8_t *)(&tmp), w->e1, 4);
	writel(tmp, &alink->tx);
	memcpy((uint8_t *)(&tmp), w->e0, 4);
	writel(tmp, &alink->tx);

	for (i = 28; i >= 0; i -= 4) {
		memcpy((uint8_t *)(&tmp), w->data + i, 4);
		writel(tmp, &alink->tx);
	}

	memcpy((uint8_t *)(&tmp), w->a2, 4);
	writel(tmp, &alink->tx);

	return 0;
}

void alink_read_result(struct result *r)
{
	uint32_t tmp;

	tmp = readl(&alink->rx);
	memcpy(r->miner_id, (uint8_t *)(&tmp), 4);

	tmp = readl(&alink->rx);
	memcpy(r->task_id, (uint8_t *)(&tmp), 4);

	tmp = readl(&alink->rx);
	memcpy(r->task_id + 4, (uint8_t *)(&tmp), 4);

	tmp = readl(&alink->rx);
	memcpy(r->timeout, (uint8_t *)(&tmp), 4);

	tmp = readl(&alink->rx);
	memcpy(r->nonce, (uint8_t *)(&tmp), 4);
}


extern void delay(unsigned int ms);
void alink_flush_fifo()
{
	uint32_t value = readl(&alink->state);
	value |= LM32_ALINK_STATE_FLUSH;
	writel(value, &alink->state);

	delay(1);
}

unsigned int asic_test_a3240_work(int core)//0 to 767
{
	uint32_t msg_blk[23], exp_nonce, send_nonce;
	uint32_t core_big = core&0xfffff0, core_small = core&0xf;
	int i;
	if(core_small == 0){
		msg_blk[ 6] = 0x4ac1d001;
		msg_blk[ 7] = 0x89517050;
		msg_blk[ 8] = 0x087e051a;
		msg_blk[ 9] = 0x06b168ae;
		msg_blk[10] = 0x62a5f25c;
		msg_blk[11] = 0x00639107;
		msg_blk[12] = 0x13cdfd7b;
		msg_blk[13] = 0xfa77fe7d;
		msg_blk[14] = 0x9cb18a17;
		msg_blk[15] = 0x65c90d1e;
		msg_blk[16] = 0x8f41371d;
		msg_blk[17] = 0x974bf4bb;
		msg_blk[18] = 0x7145fd6d;
		msg_blk[19] = 0xc44192c0;
		msg_blk[20] = 0x12146495;
		msg_blk[21] = 0xd8f8ef67;
		msg_blk[22] = 0xa2cb45c1;
		send_nonce  = 0x1bee2ba0;
	} else if(core_small == 1) {
		msg_blk[ 6] = 0xe8dc86b1;
		msg_blk[ 7] = 0x2d547050;
		msg_blk[ 8] = 0x087e051a;
		msg_blk[ 9] = 0x6e18f645;
		msg_blk[10] = 0x6d0fd9da;
		msg_blk[11] = 0xdac8ce29;
		msg_blk[12] = 0x1c9fc4ed;
		msg_blk[13] = 0x8119ce2a;
		msg_blk[14] = 0x983dab15;
		msg_blk[15] = 0x33cde02a;
		msg_blk[16] = 0x528caf7e;
		msg_blk[17] = 0xeea79cd6;
		msg_blk[18] = 0x7848c34b;
		msg_blk[19] = 0x222e8b87;
		msg_blk[20] = 0x25a54bbe;
		msg_blk[21] = 0xe06e5fab;
		msg_blk[22] = 0x9d8d4242;
		send_nonce  = 0xc168f161;
	} else if(core_small == 2) {
                msg_blk[ 6] = 0x826cce7a;
                msg_blk[ 7] = 0x3c537050;
                msg_blk[ 8] = 0x087e051a;
                msg_blk[ 9] = 0xc35604f4;
                msg_blk[10] = 0xb0496c6a;
                msg_blk[11] = 0xc5e7acf1;
                msg_blk[12] = 0xf8941ad5;
                msg_blk[13] = 0xa1d03421;
                msg_blk[14] = 0x612fc425;
                msg_blk[15] = 0x4064bca0;
                msg_blk[16] = 0x80a27591;
                msg_blk[17] = 0xf81e2a4b;
                msg_blk[18] = 0x5600f2af;
                msg_blk[19] = 0xd962d729;
                msg_blk[20] = 0x879a9334;
                msg_blk[21] = 0x2a54a40c;
                msg_blk[22] = 0xb6327d35;
                send_nonce  = 0x84aad522;
	} else if(core_small == 3) {
                msg_blk[ 6] = 0x826cce7a;
                msg_blk[ 7] = 0x3e537050;
                msg_blk[ 8] = 0x087e051a;
                msg_blk[ 9] = 0xc55604f4;
                msg_blk[10] = 0xb0496c6a;
                msg_blk[11] = 0xc3efecf2;
                msg_blk[12] = 0xfa941ad5;
                msg_blk[13] = 0xa1d03421;
                msg_blk[14] = 0x612fc425;
                msg_blk[15] = 0x4064bca0;
                msg_blk[16] = 0x80a27591;
                msg_blk[17] = 0xf81e2a4b;
                msg_blk[18] = 0x5600f2af;
                msg_blk[19] = 0xd962d729;
                msg_blk[20] = 0x879a9334;
                msg_blk[21] = 0x2a54a40c;
                msg_blk[22] = 0xb3baed4e;
                send_nonce  = 0x1c12df93;
	} else if(core_small == 4) {
                msg_blk[ 6] = 0xc290e5a7;
                msg_blk[ 7] = 0x70527050;
                msg_blk[ 8] = 0x087e051a;
                msg_blk[ 9] = 0x5e2e867c;
                msg_blk[10] = 0x431284bc;
                msg_blk[11] = 0xc6e2feca;
                msg_blk[12] = 0x65d344fe;
                msg_blk[13] = 0xa1235784;
                msg_blk[14] = 0xf1d2de33;
                msg_blk[15] = 0xed14a977;
                msg_blk[16] = 0x67eb9370;
                msg_blk[17] = 0x8a776518;
                msg_blk[18] = 0x857ccb58;
                msg_blk[19] = 0xea65fc9d;
                msg_blk[20] = 0xdd3da630;
                msg_blk[21] = 0x81c9ffff;
                msg_blk[22] = 0xcb517d22;
                send_nonce  = 0xad267354;
	} else if(core_small == 5) {
                msg_blk[ 6] = 0x4ac1d001;
                msg_blk[ 7] = 0x8f517050;
                msg_blk[ 8] = 0x087e051a;
                msg_blk[ 9] = 0x0cb168ae;
                msg_blk[10] = 0x62a5f25c;
                msg_blk[11] = 0x068a510a;
                msg_blk[12] = 0x19cdfd7b;
                msg_blk[13] = 0xfa77fe7d;
                msg_blk[14] = 0x9cb18a17;
                msg_blk[15] = 0x65c90d1e;
                msg_blk[16] = 0x8f41371d;
                msg_blk[17] = 0x974bf4bb;
                msg_blk[18] = 0x7145fd6d;
                msg_blk[19] = 0xc44192c0;
                msg_blk[20] = 0x12146495;
                msg_blk[21] = 0xd8f8ef67;
                msg_blk[22] = 0xb171b59c;
                send_nonce  = 0xdaa43ad5;
	} else if(core_small == 6) {
                msg_blk[ 6] = 0x4ac1d001;
                msg_blk[ 7] = 0x88517050;
                msg_blk[ 8] = 0x087e051a;
                msg_blk[ 9] = 0x05b168ae;
                msg_blk[10] = 0x62a5f25c;
                msg_blk[11] = 0x81677107;
                msg_blk[12] = 0x12cdfd7b;
                msg_blk[13] = 0xfa77fe7d;
                msg_blk[14] = 0x9cb18a17;
                msg_blk[15] = 0x65c90d1e;
                msg_blk[16] = 0x8f41371d;
                msg_blk[17] = 0x974bf4bb;
                msg_blk[18] = 0x7145fd6d;
                msg_blk[19] = 0xc44192c0;
                msg_blk[20] = 0x12146495;
                msg_blk[21] = 0xd8f8ef67;
                msg_blk[22] = 0x220f1dbd;
                send_nonce  = 0x010ebeb6;
	} else if(core_small == 7) {
                msg_blk[ 6] = 0xc290e5a7;
                msg_blk[ 7] = 0x6f527050;
                msg_blk[ 8] = 0x087e051a;
                msg_blk[ 9] = 0x5d2e867c;
                msg_blk[10] = 0x431284bc;
                msg_blk[11] = 0x46e71eca;
                msg_blk[12] = 0x64d344fe;
                msg_blk[13] = 0xa1235784;
                msg_blk[14] = 0xf1d2de33;
                msg_blk[15] = 0xed14a977;
                msg_blk[16] = 0x67eb9370;
                msg_blk[17] = 0x8a776518;
                msg_blk[18] = 0x857ccb58;
                msg_blk[19] = 0xea65fc9d;
                msg_blk[20] = 0xdd3da630;
                msg_blk[21] = 0x81c9ffff;
                msg_blk[22] = 0x4995b52e;
                send_nonce  = 0x3db0de27;
	} else if(core_small == 8) {
		msg_blk[ 6] = 0x9ed94986;
		msg_blk[ 7] = 0x13557050;
		msg_blk[ 8] = 0x087e051a;
		msg_blk[ 9] = 0x39171075;
		msg_blk[10] = 0xf55f91a6;
		msg_blk[11] = 0x67d1e0c0;
		msg_blk[12] = 0x24d6e2cd;
		msg_blk[13] = 0x92f7791a;
		msg_blk[14] = 0xcc39201c;
		msg_blk[15] = 0xc572d901;
		msg_blk[16] = 0xff62389c;
		msg_blk[17] = 0x42727a8e;
		msg_blk[18] = 0xc35ad8db;
		msg_blk[19] = 0x19fae6f7;
		msg_blk[20] = 0xe2e8080f;
		msg_blk[21] = 0x740d56be;
		msg_blk[22] = 0x312bf034;
		send_nonce  = 0x62b04f08;
	} else if(core_small == 9) {
                msg_blk[ 6] = 0xc290e5a7;
                msg_blk[ 7] = 0x6b527050;
                msg_blk[ 8] = 0x087e051a;
                msg_blk[ 9] = 0x592e867c;
                msg_blk[10] = 0x431284bc;
                msg_blk[11] = 0x4ad69ecc;
                msg_blk[12] = 0x60d344fe;
                msg_blk[13] = 0xa1235784;
                msg_blk[14] = 0xf1d2de33;
                msg_blk[15] = 0xed14a977;
                msg_blk[16] = 0x67eb9370;
                msg_blk[17] = 0x8a776518;
                msg_blk[18] = 0x857ccb58;
                msg_blk[19] = 0xea65fc9d;
                msg_blk[20] = 0xdd3da630;
                msg_blk[21] = 0x81c9ffff;
                msg_blk[22] = 0x4e851520;
                send_nonce  = 0x10f79f79;
	} else if(core_small == 0xa) {
                msg_blk[ 6] = 0x4ac1d001;
                msg_blk[ 7] = 0x8c517050;
                msg_blk[ 8] = 0x087e051a;
                msg_blk[ 9] = 0x09b168ae;
                msg_blk[10] = 0x62a5f25c;
                msg_blk[11] = 0x8156f105;
                msg_blk[12] = 0x16cdfd7b;
                msg_blk[13] = 0xfa77fe7d;
                msg_blk[14] = 0x9cb18a17;
                msg_blk[15] = 0x65c90d1e;
                msg_blk[16] = 0x8f41371d;
                msg_blk[17] = 0x974bf4bb;
                msg_blk[18] = 0x7145fd6d;
                msg_blk[19] = 0xc44192c0;
                msg_blk[20] = 0x12146495;
                msg_blk[21] = 0xd8f8ef67;
                msg_blk[22] = 0x26fe7d8b;
                send_nonce  = 0xc95d38ca;
	} else if(core_small == 0xb) {
                msg_blk[ 6] = 0x826cce7a;
                msg_blk[ 7] = 0x42537050;
                msg_blk[ 8] = 0x087e051a;
                msg_blk[ 9] = 0xc95604f4;
                msg_blk[10] = 0xb0496c6a;
                msg_blk[11] = 0xbfe06cf4;
                msg_blk[12] = 0xfe941ad5;
                msg_blk[13] = 0xa1d03421;
                msg_blk[14] = 0x612fc425;
                msg_blk[15] = 0x4064bca0;
                msg_blk[16] = 0x80a27591;
                msg_blk[17] = 0xf81e2a4b;
                msg_blk[18] = 0x5600f2af;
                msg_blk[19] = 0xd962d729;
                msg_blk[20] = 0x879a9334;
                msg_blk[21] = 0x2a54a40c;
                msg_blk[22] = 0xaeab0d60;
                send_nonce  = 0x508fae1b;
	} else if(core_small == 0xc) {
                msg_blk[ 6] = 0xe8dc86b1;
                msg_blk[ 7] = 0x34547050;
                msg_blk[ 8] = 0x087e051a;
                msg_blk[ 9] = 0x7518f645;
                msg_blk[10] = 0x6d0fd9da;
                msg_blk[11] = 0x545dae3e;
                msg_blk[12] = 0x239fc4ed;
                msg_blk[13] = 0x8119ce2a;
                msg_blk[14] = 0x983dab15;
                msg_blk[15] = 0x33cde02a;
                msg_blk[16] = 0x528caf7e;
                msg_blk[17] = 0xeea79cd6;
                msg_blk[18] = 0x7848c34b;
                msg_blk[19] = 0x222e8b87;
                msg_blk[20] = 0x25a54bbe;
                msg_blk[21] = 0xe06e5fab;
                msg_blk[22] = 0x2a6249eb;
                send_nonce  = 0x93762bfc;
	} else if(core_small == 0xd) {
                msg_blk[ 6] = 0x826cce7a;
                msg_blk[ 7] = 0x45537050;
                msg_blk[ 8] = 0x087e051a;
                msg_blk[ 9] = 0xcc5604f4;
                msg_blk[10] = 0xb0496c6a;
                msg_blk[11] = 0xf4428d45;
                msg_blk[12] = 0x01941ad5;
                msg_blk[13] = 0xa1d03421;
                msg_blk[14] = 0x612fc425;
                msg_blk[15] = 0x4064bca0;
                msg_blk[16] = 0x80a27591;
                msg_blk[17] = 0xf81e2a4b;
                msg_blk[18] = 0x5600f2af;
                msg_blk[19] = 0xd962d729;
                msg_blk[20] = 0x879a9334;
                msg_blk[21] = 0x2a54a40c;
                msg_blk[22] = 0xe0cd45c5;
                send_nonce  = 0xc803d2fd;
	} else if(core_small == 0xe) {
                msg_blk[ 6] = 0xc290e5a7;
                msg_blk[ 7] = 0x65527050;
                msg_blk[ 8] = 0x087e051a;
                msg_blk[ 9] = 0x532e867c;
                msg_blk[10] = 0x431284bc;
                msg_blk[11] = 0x29ebded7;
                msg_blk[12] = 0x5ad344fe;
                msg_blk[13] = 0xa1235784;
                msg_blk[14] = 0xf1d2de33;
                msg_blk[15] = 0xed14a977;
                msg_blk[16] = 0x67eb9370;
                msg_blk[17] = 0x8a776518;
                msg_blk[18] = 0x857ccb58;
                msg_blk[19] = 0xea65fc9d;
                msg_blk[20] = 0xdd3da630;
                msg_blk[21] = 0x81c9ffff;
                msg_blk[22] = 0x2d1a8543;
                send_nonce  = 0x873d1a5e;
	} else {//if(core_small == 0xf) {
                msg_blk[ 6] = 0x826cce7a;
                msg_blk[ 7] = 0x3b537050;
                msg_blk[ 8] = 0x087e051a;
                msg_blk[ 9] = 0xc25604f4;
                msg_blk[10] = 0xb0496c6a;
                msg_blk[11] = 0x41054cf0;
                msg_blk[12] = 0xf7941ad5;
                msg_blk[13] = 0xa1d03421;
                msg_blk[14] = 0x612fc425;
                msg_blk[15] = 0x4064bca0;
                msg_blk[16] = 0x80a27591;
                msg_blk[17] = 0xf81e2a4b;
                msg_blk[18] = 0x5600f2af;
                msg_blk[19] = 0xd962d729;
                msg_blk[20] = 0x879a9334;
                msg_blk[21] = 0x2a54a40c;
                msg_blk[22] = 0x30101538;
                send_nonce  = 0xa2bfe63f;
	}

	msg_blk[ 4] = 0x00000001;
	msg_blk[ 5] = 0x00000000;
	msg_blk[ 3] = 0x0000ffff;	/* The real timeout is 0x75d1 */
	msg_blk[ 2] = 0x24924925;	/* Step for 7 chips */
	msg_blk[ 1] = send_nonce ^ core_big;	/* Nonce start */
	msg_blk[ 0] = 0x00000000;	/* Chip index */
	exp_nonce   = send_nonce + 0x6000;
	for (i = 0; i < 23; i++) {
		writel(msg_blk[i], &alink->tx);
	}
	return exp_nonce;
}

static void asic_test_work(int chip, int core, int gate)
{
	uint32_t msg_blk[23];
	int i;

	msg_blk[22] = 0x220f1dbd;	/* a2 */

	msg_blk[21] = 0xd8f8ef67;	/* Midstat */
	msg_blk[20] = 0x12146495;
	msg_blk[19] = 0xc44192c0;
	msg_blk[18] = 0x7145fd6d;
	msg_blk[17] = 0x974bf4bb;
	msg_blk[16] = 0x8f41371d;
	msg_blk[15] = 0x65c90d1e;
	msg_blk[14] = 0x9cb18a17;

	msg_blk[13] = 0xfa77fe7d;	/* e0 */
	msg_blk[12] = 0x12cdfd7b;	/* e1 */
	msg_blk[11] = 0x81677107;	/* e2 */
	msg_blk[10] = 0x62a5f25c;	/* a0 */
	msg_blk[9]  = 0x05b168ae;	/* a1 */

	msg_blk[8]  = 0x087e051a;	/* Data */
	msg_blk[7]  = 0x88517050;
	msg_blk[6]  = 0x4ac1d001;

	msg_blk[5]  = 0x00000174;	/* Clock at 1Ghs */
	msg_blk[4]  = 0x82600000 | (gate ? 0xb : 0x7);
	msg_blk[3]  = (gate ? 0x2fffffff : 0x0000ffff);	/* The real timeout is 0x75d1 */
	msg_blk[2]  = 0x24924925;	/* Step for 7 chips */
	msg_blk[1]  = (0x010f1036 ^ core) - 5 * 128  ;	/* Nonce start, have to be N * 128 */
	msg_blk[0]  = chip;	/* Chip index */

	for (i = 0; i < 23; i++) {
		writel(msg_blk[i], &alink->tx);
	}
	/* Return nonce - 0x180 = Real: 010f0eb6 */
}

#ifdef DEBUG
void alink_buf_status()
{
	uint32_t value;

	value = readl(&alink->busy);
	debug32("Alink: B: %08x,", value);

	value = readl(&alink->state);
	debug32(" S: %08x (tx: %d, rx: %d(%d))\n",
		value,
		((value & LM32_ALINK_STATE_TXCOUNT) >> 4),
		((value & LM32_ALINK_STATE_RXCOUNT) >> 20),
		((value & LM32_ALINK_STATE_RXCOUNT) >> 20) / 5);
}
#endif

extern void send_pkg(int type, uint8_t *buf, unsigned int len);
void alink_asic_idle()
{
	int i;

	alink_flush_fifo();
	for (i = 0; i < AVA2_DEFAULT_MINERS; i++) {
		debug32("%d", i);
		alink_init(1 << i);	/* Enable i miners */
		asic_test_work(i, 0, 1);
		while (!alink_txbuf_count())
			;
		while (!alink_busy_status())
			;
		delay(1);
	}
	alink_init(0x3ff);
}

void alink_asic_test()
{
	int i, j, k, core;
	uint32_t nonce;
	struct result result;
	uint8_t core_test[ASIC_COUNT + 1];

	writel(LM32_ALINK_STATE_TEST, &alink->state); /* Enable alink scan mode */

	for (i = 0; i < AVA2_DEFAULT_MINERS; i++) {
		debug32("%d:", i);
		alink_init(1 << i);	/* Enable i miners */
		for (j = 0; j < ASIC_COUNT; j++) {
			core = 0;
			for (k = 0; k < 128; k++) {
				asic_test_work(j, k, 0);		/* Test asic cores  */

				while (!alink_txbuf_count())
					;
				while (!alink_busy_status())
					;

				delay(1);
				if (!alink_rxbuf_empty()) {
					alink_read_result(&result);
					memcpy(&nonce, result.nonce, 4);
					if (nonce != 0x010f1036)
						core++;
				} else
					core++;
			}
			core_test[j + 1] = core;
			debug32("%3d", core);
		}
		core_test[0] = i + 1;
		/* Send out one pkg */
		send_pkg(AVA2_P_TEST_RET, core_test, ASIC_COUNT + 1);
	}

	writel(0, &alink->state); /* Enable alink hash mode */
	alink_init(0x3ff);
}

void alink_a3240_test()
{
	int k, error_cnt=0, pass_cnt=0;
	uint32_t nonce, exp_nonce;
	struct result result;

	writel(LM32_ALINK_STATE_TEST, &alink->state); /* Enable alink scan mode */

	alink_init(0x1);/* Enable 0 miners */
	for (k = 0; k < 768; k++) {
		exp_nonce = asic_test_a3240_work(k);		/* Test asic cores  */
		while (!alink_txbuf_count())
			;
		while (!alink_busy_status())
			;

		delay(1);
		if (!alink_rxbuf_empty()){
			alink_read_result(&result);
			memcpy(&nonce, result.nonce, 4);
			if (nonce != exp_nonce){
				debug32("Error small core %3d\n", k);
				error_cnt++;
			} else {
				debug32("Pass small core %3d\n", k);
				pass_cnt++;
			}
		} else {
			debug32("Error small core %3d\n", k);
			error_cnt++;
		}
	}

	 debug32("Pass core number %3d\n", pass_cnt);
	 debug32("Error core number %3d\n", error_cnt);
}
