/*
 * Author: Xiangfu Liu <xiangfu@openmobilefree.net>
 * Bitcoin:	1CanaaniJzgps8EV6Sfmpb7T8RutpaeyFn
 *
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */

#include <stdbool.h>
#include <stdint.h>

#include "minilibc.h"
#include "system_config.h"
#include "defines.h"
#include "io.h"
#include "uart.h"
#include "miner.h"
#include "sha256.h"
#include "alink.h"
#include "twipwm.h"

static uint32_t g_asic_freq = ASIC_FREQUENCY;

static void flip32(void *dest_p, const void *src_p)
{
	uint32_t *dest = dest_p;
	const uint32_t *src = src_p;
	int i;

	for (i = 0; i < 8; i++)
		dest[i] = bswap_32(src[i]);
}

static void flip64(void *dest_p, const uint8_t *src_p)
{
	uint32_t *dest = dest_p;
	int i;

	for (i = 0; i < 16; i++) {
		dest[i] = src_p[i * 4 + 0] | src_p[i * 4 + 1] << 8 |
			src_p[i * 4 + 2] << 16 | src_p[i * 4 + 3] << 24;
	}
}

static void gen_hash(uint8_t *data, uint8_t *hash, unsigned int len)
{
	uint8_t hash1[32];

	sha256(data, len, hash1);
	sha256(hash1, 32, hash);
}

static void calc_midstate(struct mm_work *mw, struct work *work)
{
	unsigned char data[64];
	uint32_t *data32 = (uint32_t *)data;

	flip64(data32, mw->header);

	sha256_init();
	sha256_update(data, 64);
	sha256_final(work->data);

	/* FIXME: LM32 will crash if I direct use
	 * flip64(work->data, work->data);
	 * Should be flip32 ??
	 */
	memcpy(data, work->data, 32);
	flip64(data32, data);

	memcpy(work->data, data, 32);
	memcpy(work->data + 32, mw->header + 64, 12);
}

void set_asic_freq(uint32_t value)
{
	g_asic_freq = value;
}

uint32_t get_asic_freq()
{
	return g_asic_freq;
}

void miner_init_work(struct mm_work *mw, struct work *work)
{
	int timeout;

	memcpy(work->task_id, (uint8_t *)(&mw->pool_no), 4);
	memcpy(work->task_id + 4, (uint8_t *)(&work->nonce2), 4);

	timeout = 4294967 / (g_asic_freq * ASIC_COUNT); /* Time in ms */
	timeout *= CPU_FREQUENCY / 1000;     /* Time in cpu clock */
	memcpy(work->timeout, &timeout, 4);

	switch (g_asic_freq / 2) { /* This is the real clock in Mhz, 1Mhz means 2Mhs */
	case 1000:
		work->clock[1] = 0xe0;
		work->clock[0] = 0x84;
		break;
	case 950:
		work->clock[1] = 0xa0;
		work->clock[0] = 0x84;
		break;
	case 900:
		work->clock[1] = 0x60;
		work->clock[0] = 0x84;
		break;
	case 850:
		work->clock[1] = 0x20;
		work->clock[0] = 0x84;
		break;
	case 800:
		work->clock[1] = 0xe0;
		work->clock[0] = 0x83;
		break;
	case 750:
		work->clock[1] = 0xa0;
		work->clock[0] = 0x83;
		break;
	case 700:
		work->clock[1] = 0x60;
		work->clock[0] = 0x83;
		break;
	case 650:
		work->clock[1] = 0x20;
		work->clock[0] = 0x83;
		break;
	case 600:
		work->clock[1] = 0xe0;
		work->clock[0] = 0x82;
		break;
	case 550:
		work->clock[1] = 0xa0;
		work->clock[0] = 0x82;
		break;
	default:        /* 500Mhz etc */
		work->clock[1] = 0xe0;
		work->clock[0] = 0x94;
		break;
	}
	work->clock[2] = 0x00;
	work->clock[3] = 0x07;	/* 0x0b: idle, 0x07: enable */

	work->clock[4] = 0x00;
	work->clock[5] = 0x00;
	work->clock[6] = 0x01;
	work->clock[7] = 0x74;

	work->step[0] = 0x24;	/* 6: 0x2aaaaaaa, 7: 0x24924924, 10: 0x19999999 */
	work->step[1] = 0x92;
	work->step[2] = 0x49;
	work->step[3] = 0x24;
}

static void rev(unsigned char *s, size_t l)
{
	size_t i, j;
	unsigned char t;

	for (i = 0, j = l - 1; i < j; i++, j--) {
		t = s[i];
		s[i] = s[j];
		s[j] = t;
	}
}

/* Total: 4W + 19W = 23W
 * TaskID_H:1, TASKID_L:1, STEP:1, TIMEOUT:1,
 * CLK_CFG:2, a2, Midsate:8, e0, e1, e2, a0, a1, Data:3
 */
#ifdef HW_PRE_CALC
static void calc_prepare(struct work *work, uint8_t *buf)
{
	uint32_t precalc[6];
	sha256_precalc(buf, buf + 32, 44, (uint8_t *)precalc);
	memcpy(work->a0, precalc + 0, 4);
	memcpy(work->a1, precalc + 1, 4);
	memcpy(work->a2, precalc + 2, 4);
	memcpy(work->e0, precalc + 3, 4);
	memcpy(work->e1, precalc + 4, 4);
	memcpy(work->e2, precalc + 5, 4);
}
#endif

extern void calc_prepare1(struct work *work, uint8_t *buf);
void miner_gen_nonce2_work(struct mm_work *mw, uint32_t nonce2, struct work *work)
{
	uint8_t merkle_root[32], merkle_sha[64];
	uint32_t *data32, *swap32, tmp32;
	int i;

	tmp32 = bswap_32(nonce2);
	memcpy(mw->coinbase + mw->nonce2_offset, (uint8_t *)(&tmp32), sizeof(uint32_t));
	work->nonce2 = nonce2;

	gen_hash(mw->coinbase, merkle_root, mw->coinbase_len);
	memcpy(merkle_sha, merkle_root, 32);
	for (i = 0; i < mw->nmerkles; i++) {
		memcpy(merkle_sha + 32, mw->merkles[i], 32);
		gen_hash(merkle_sha, merkle_root, 64);
		memcpy(merkle_sha, merkle_root, 32);
	}
	data32 = (uint32_t *)merkle_sha;
	swap32 = (uint32_t *)merkle_root;
	flip32(swap32, data32);

	memcpy(mw->header + mw->merkle_offset, merkle_root, 32);

	debug32("D: Work nonce2: %08x\n", work->nonce2);
	calc_midstate(mw, work);

	uint8_t work_t[44];
#ifdef HW_PRE_CALC
	uint8_t work_t[] = {
		0x05, 0x4e, 0x53, 0xc3, 0xc4, 0xd4, 0xba, 0x3e, 0x65, 0x40, 0x99, 0x4f, 0x06, 0x67, 0x91, 0x31,
		0xa7, 0x2d, 0x66, 0xaa, 0x68, 0x4f, 0x0e, 0xdb, 0xc3, 0x6d, 0x95, 0x8a, 0x46, 0x6e, 0x4d, 0xb2,
		0x1c, 0x26, 0x52, 0xfb, 0x52, 0xa0, 0x26, 0xf4, 0x19, 0x06, 0x12, 0x42};
#endif

	memcpy(work_t, work->data, 44);
	rev(work_t, 32);
	rev(work_t + 32, 12);
	memcpy(work->data, work_t, 44);

#ifdef HW_PRE_CALC
	calc_prepare(work, work_t);
	memcpy((uint8_t *)(&tmp32), work->a1, 4);
	debug32("%08x,", tmp32);
	memcpy((uint8_t *)(&tmp32), work->a0, 4);
	debug32("%08x,", tmp32);
	memcpy((uint8_t *)(&tmp32), work->e2, 4);
	debug32("%08x,", tmp32);
	memcpy((uint8_t *)(&tmp32), work->e1, 4);
	debug32("%08x,", tmp32);
	memcpy((uint8_t *)(&tmp32), work->e0, 4);
	debug32("%08x,", tmp32);
	memcpy((uint8_t *)(&tmp32), work->a2, 4);
	debug32("%08x", tmp32);
	debug32("\n");
#endif

	calc_prepare1(work, work->data);
	memcpy((uint8_t *)(&tmp32), work->a1, 4);
	memcpy((uint8_t *)(&tmp32), work->a0, 4);
	memcpy((uint8_t *)(&tmp32), work->e2, 4);
	memcpy((uint8_t *)(&tmp32), work->e1, 4);
	memcpy((uint8_t *)(&tmp32), work->e0, 4);
	memcpy((uint8_t *)(&tmp32), work->a2, 4);
}
