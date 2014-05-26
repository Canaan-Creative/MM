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
#include "timer.h"

#if defined(AVALON2_A3255_MACHINE)
  #define MODULE_ENABLE 0x3ff
#elif defined(AVALON3_A3233_MACHINE)
  #define MODULE_ENABLE 0x1f
#elif defined(AVALON3_A3233_CARD)
  #define MODULE_ENABLE 0xf
#endif

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

#if defined(AVALON2_A3255_MACHINE)
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

	msg_blk[5]  = 0x00000000;	/* Default Clock */

	msg_blk[4]  = 0x82600000 | (gate ? 0xb : 0x7);
	msg_blk[3]  = (gate ? 0x2fffffff : 0x0000ffff);	/* The real timeout is 0x75d1 */
	msg_blk[2]  = 0x24924925;	/* Step for 7 chips */
	msg_blk[1]  = (0x010f1036 ^ core) - 5 * 128;
	msg_blk[0]  = chip;		/* Chip index */

	for (i = 0; i < 23; i++) {
		writel(msg_blk[i], &alink->tx);
	}
}
#elif defined(AVALON3_A3233_MACHINE)
const uint32_t test_data[16][18] = {
	{0x4ac1d001, 0x89517050, 0x087e051a, 0x06b168ae,
	 0x62a5f25c, 0x00639107, 0x13cdfd7b, 0xfa77fe7d,
	 0x9cb18a17, 0x65c90d1e, 0x8f41371d, 0x974bf4bb,
	 0x7145fd6d, 0xc44192c0, 0x12146495, 0xd8f8ef67,
	 0xa2cb45c1, 0x1bee2ba0}, 

	{0xe8dc86b1, 0x2d547050, 0x087e051a, 0x6e18f645,
	 0x6d0fd9da, 0xdac8ce29, 0x1c9fc4ed, 0x8119ce2a,
	 0x983dab15, 0x33cde02a, 0x528caf7e, 0xeea79cd6,
	 0x7848c34b, 0x222e8b87, 0x25a54bbe, 0xe06e5fab,
	 0x9d8d4242, 0xc168f161}, 

	{0x826cce7a, 0x3c537050, 0x087e051a, 0xc35604f4,
	 0xb0496c6a, 0xc5e7acf1, 0xf8941ad5, 0xa1d03421,
	 0x612fc425, 0x4064bca0, 0x80a27591, 0xf81e2a4b,
	 0x5600f2af, 0xd962d729, 0x879a9334, 0x2a54a40c,
	 0xb6327d35,  0x84aad522},

	{0x826cce7a, 0x3e537050, 0x087e051a, 0xc55604f4,
	 0xb0496c6a, 0xc3efecf2, 0xfa941ad5, 0xa1d03421,
	 0x612fc425, 0x4064bca0, 0x80a27591, 0xf81e2a4b,
	 0x5600f2af, 0xd962d729, 0x879a9334, 0x2a54a40c,
	 0xb3baed4e, 0x1c12df93},

	{0xc290e5a7, 0x70527050, 0x087e051a, 0x5e2e867c,
	 0x431284bc, 0xc6e2feca, 0x65d344fe, 0xa1235784,
	 0xf1d2de33, 0xed14a977, 0x67eb9370, 0x8a776518,
	 0x857ccb58, 0xea65fc9d, 0xdd3da630, 0x81c9ffff,
	 0xcb517d22, 0xad267354},

	{0x4ac1d001, 0x8f517050, 0x087e051a, 0x0cb168ae,
	 0x62a5f25c, 0x068a510a, 0x19cdfd7b, 0xfa77fe7d,
	 0x9cb18a17, 0x65c90d1e, 0x8f41371d, 0x974bf4bb,
	 0x7145fd6d, 0xc44192c0, 0x12146495, 0xd8f8ef67,
	 0xb171b59c, 0xdaa43ad5},

	{0x4ac1d001, 0x88517050, 0x087e051a, 0x05b168ae,
	 0x62a5f25c, 0x81677107, 0x12cdfd7b, 0xfa77fe7d,
	 0x9cb18a17, 0x65c90d1e, 0x8f41371d, 0x974bf4bb,
	 0x7145fd6d, 0xc44192c0, 0x12146495, 0xd8f8ef67,
	 0x220f1dbd, 0x010ebeb6},

	{0xc290e5a7, 0x6f527050, 0x087e051a, 0x5d2e867c,
	 0x431284bc, 0x46e71eca, 0x64d344fe, 0xa1235784,
	 0xf1d2de33, 0xed14a977, 0x67eb9370, 0x8a776518,
	 0x857ccb58, 0xea65fc9d, 0xdd3da630, 0x81c9ffff,
	 0x4995b52e, 0x3db0de27},

	{0x9ed94986, 0x13557050, 0x087e051a, 0x39171075,
	 0xf55f91a6, 0x67d1e0c0, 0x24d6e2cd, 0x92f7791a,
	 0xcc39201c, 0xc572d901, 0xff62389c, 0x42727a8e,
	 0xc35ad8db, 0x19fae6f7, 0xe2e8080f, 0x740d56be, 
	 0x312bf034, 0x62b04f08},

	{0xc290e5a7, 0x6b527050, 0x087e051a, 0x592e867c,
	 0x431284bc, 0x4ad69ecc, 0x60d344fe, 0xa1235784,
	 0xf1d2de33, 0xed14a977, 0x67eb9370, 0x8a776518,
	 0x857ccb58, 0xea65fc9d, 0xdd3da630, 0x81c9ffff,
	 0x4e851520, 0x10f79f79},

	{0x4ac1d001, 0x8c517050, 0x087e051a, 0x09b168ae,
	 0x62a5f25c, 0x8156f105, 0x16cdfd7b, 0xfa77fe7d,
	 0x9cb18a17, 0x65c90d1e, 0x8f41371d, 0x974bf4bb,
	 0x7145fd6d, 0xc44192c0, 0x12146495, 0xd8f8ef67,
	 0x26fe7d8b, 0xc95d38ca},

	{0x826cce7a, 0x42537050, 0x087e051a, 0xc95604f4,
	 0xb0496c6a, 0xbfe06cf4, 0xfe941ad5, 0xa1d03421,
	 0x612fc425, 0x4064bca0, 0x80a27591, 0xf81e2a4b,
	 0x5600f2af, 0xd962d729, 0x879a9334, 0x2a54a40c,
	 0xaeab0d60, 0x508fae1b},

	{0xe8dc86b1, 0x34547050, 0x087e051a, 0x7518f645,
	 0x6d0fd9da, 0x545dae3e, 0x239fc4ed, 0x8119ce2a,
	 0x983dab15, 0x33cde02a, 0x528caf7e, 0xeea79cd6,
	 0x7848c34b, 0x222e8b87, 0x25a54bbe, 0xe06e5fab,
	 0x2a6249eb, 0x93762bfc},

	{0x826cce7a, 0x45537050, 0x087e051a, 0xcc5604f4,
	 0xb0496c6a, 0xf4428d45, 0x01941ad5, 0xa1d03421,
	 0x612fc425, 0x4064bca0, 0x80a27591, 0xf81e2a4b,
	 0x5600f2af, 0xd962d729, 0x879a9334, 0x2a54a40c,
	 0xe0cd45c5, 0xc803d2fd},

	{0xc290e5a7, 0x65527050, 0x087e051a, 0x532e867c,
	 0x431284bc, 0x29ebded7, 0x5ad344fe, 0xa1235784,
	 0xf1d2de33, 0xed14a977, 0x67eb9370, 0x8a776518,
	 0x857ccb58, 0xea65fc9d, 0xdd3da630, 0x81c9ffff,
	 0x2d1a8543, 0x873d1a5e},

	{0x826cce7a, 0x3b537050, 0x087e051a, 0xc25604f4,
	 0xb0496c6a, 0x41054cf0, 0xf7941ad5, 0xa1d03421,
	 0x612fc425, 0x4064bca0, 0x80a27591, 0xf81e2a4b,
	 0x5600f2af, 0xd962d729, 0x879a9334, 0x2a54a40c,
	 0x30101538, 0xa2bfe63f}
};

static uint32_t asic_test_work(int chip, int core, int gate)
{
	uint32_t msg_blk[23], send_nonce;
	uint32_t core_big = core & 0xfffff0, core_small = core & 0xf;
	int i;

	for (i = 0; i < 17; i++)
		msg_blk[6 + i] = test_data[core_small][i];

	send_nonce  = test_data[core_small][17];

	msg_blk[4] = 0x00000000 | (gate ? 0xb : 0x1);
	msg_blk[3] = (gate ? 0x2fffffff : 0x0000ffff);	/* The real timeout is 0x75d1 */
	msg_blk[2] = 0x1999999a;	/* Step for 10 chips */
	msg_blk[1] = send_nonce ^ core_big;	/* Nonce start */
	msg_blk[0] = chip;		/* Chip index */

	for (i = 0; i < 23; i++) {
		writel(msg_blk[i], &alink->tx);
	}

	return send_nonce + 0x6000;
}
#endif

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
	alink_init(MODULE_ENABLE);
}

void alink_asic_test(int core_start, int core_end, int full_test)
{
	int i, j, k, core;
	uint32_t nonce;
	struct result result;
	uint8_t core_test[ASIC_COUNT + 1];
	int error = 0;

	writel(LM32_ALINK_STATE_TEST, &alink->state); /* Enable alink scan mode */

	for (i = 0; i < AVA2_DEFAULT_MINERS; i++) {
		debug32("%d:", i);
		alink_init(1 << i);	/* Enable i miners */
		for (j = 0; j < ASIC_COUNT; j++) {
			core = 0;
			for (k = core_start; k < core_end; k++) {
#if defined(AVALON2_A3255_MACHINE)
				asic_test_work(j, k, 0);
#elif defined(AVALON3_A3233_MACHINE)
				uint32_t exp_nonce;
				exp_nonce = asic_test_work(j, k, 0);
#endif

				while (!alink_txbuf_count())
					;
				while (!alink_busy_status())
					;

				delay(1);
				if (!alink_rxbuf_empty()) {
					alink_read_result(&result);
					memcpy(&nonce, result.nonce, 4);
#if defined(AVALON2_A3255_MACHINE)
					if (nonce != 0x010f1036) {
#elif defined(AVALON3_A3233_MACHINE)
					if (nonce != exp_nonce) {
#endif
						core++;
					}
				} else
					core++;
			}
			core_test[j + 1] = core >= 255 ? 255 : core;
			debug32("%3d", core);
		}
		core_test[0] = i + 1;

		if (full_test) {
			/* Send out one pkg */
			send_pkg(AVA2_P_TEST_RET, core_test, ASIC_COUNT + 1);
		} else {
			for (j = 0; j < ASIC_COUNT; j++) {
				if (core_test[j + 1] >= core_end - core_start)
					error++;
			}
		}
	}

	if (!full_test && (error < 2)) {
#if defined(AVALON3_A3233_MACHINE) || defined(AVALON3_A3233_CARD)
		led(2);
#else
		led(0);
#endif
	}

	writel(0, &alink->state); /* Enable alink hash mode */
	alink_init(MODULE_ENABLE);
}
