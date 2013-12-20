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
	debug32("D: tx full %d\n", value);
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

#ifdef DEBUG
void send_test_work(int value)
{
	uint32_t msg_blk[23];
	int i;

	debug32("Send test task:\n");
	msg_blk[22]=0x220f1dbd;	/* a2 */

	msg_blk[21]=0xd8f8ef67;	/* Midstat */
	msg_blk[20]=0x12146495;
	msg_blk[19]=0xc44192c0;
	msg_blk[18]=0x7145fd6d;
	msg_blk[17]=0x974bf4bb;
	msg_blk[16]=0x8f41371d;
	msg_blk[15]=0x65c90d1e;
	msg_blk[14]=0x9cb18a17;

	msg_blk[13]=0xfa77fe7d;	/* e0 */
	msg_blk[12]=0x12cdfd7b;	/* e1 */
	msg_blk[11]=0x81677107;	/* e2 */
	msg_blk[10]=0x62a5f25c;	/* a0 */
	msg_blk[9] =0x05b168ae;	/* a1 */

	msg_blk[8] =0x087e051a;	/* Data */
	msg_blk[7] =0x88517050;
	msg_blk[6] =0x4ac1d001;

	msg_blk[5] =0x00000000; /* Clock cfg1 */
	msg_blk[4] =0x00000001; /* Clock cfg0 */
	msg_blk[3] =0x2aaaaaaa; /* Timeout */
	msg_blk[2] =0x19999999; /* Step */
	msg_blk[1] =value;	/* Task ID H */
	msg_blk[0] =0xaabbccdd; /* Task ID L */

	for (i = 0; i < 23; i++) {
		writel(msg_blk[i], &alink->tx);
	}
	/* Nonce: 010f0eb6 */
}

void alink_buf_status()
{
	uint32_t value;

	value = readl(&alink->busy);
	debug32("D: PHY busy: %08x,", value);

	value = readl(&alink->state);
	debug32(" state: %08x (tx: %d, rx: %d(%d))\n",
		value,
		((value & LM32_ALINK_STATE_TXCOUNT) >> 4),
		((value & LM32_ALINK_STATE_RXCOUNT) >> 20),
		((value & LM32_ALINK_STATE_RXCOUNT) >> 20) / 5);
}
#endif

