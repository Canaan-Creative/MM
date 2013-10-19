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
#include "serial.h"
#include "miner.h"
#include "sha256.h"
#include "alink.h"
#include "twipwm.h"

static void flip32(void *dest_p, const void *src_p)
{
	uint32_t *dest = dest_p;
	const uint32_t *src = src_p;
	int i;

	for (i = 0; i < 8; i++)
		dest[i] = bswap_32(src[i]);
}

static void flip64(void *dest_p, const void *src_p)
{
	uint32_t *dest = dest_p;
	const uint32_t *src = src_p;
	int i;

	for (i = 0; i < 16; i++)
		dest[i] = bswap_32(src[i]);
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

	flip64(work->data, work->data);
	memcpy(work->data + 32, mw->header + 64, 12);
}

/* Total: 4W + 19W = 23W
 * TaskID_H:1, TASKID_L:1, STEP:1, TIMEOUT:1,
 * CLK_CFG:2, a2, Midsate:8, e0, e1, e2, a0, a1, Data:3
 */
static void calc_prepare(struct mm_work *mw, struct work *work)
{
	uint32_t precalc[6];
	sha256_precalc(work->data, 44, (uint8_t *)precalc);
	memcpy(work->a0, precalc + 0, 4);
	memcpy(work->a1, precalc + 1, 4);
	memcpy(work->a2, precalc + 2, 4);
	memcpy(work->e0, precalc + 3, 4);
	memcpy(work->e1, precalc + 4, 4);
	memcpy(work->e2, precalc + 5, 4);
}

void miner_init_work(struct mm_work *mw, struct work *work)
{
	/* TODO: create the task_id */
	work->task_id[0] = 0x55;
	work->task_id[1] = 0xaa;
	work->task_id[2] = 0x66;
	work->task_id[3] = 0xbb;
	memcpy(work->task_id + 4, (uint8_t *)(&work->nonce2), 4);

	work->timeout[0] = 0x04;
	work->timeout[1] = 0xfa;
	work->timeout[2] = 0x1b;
	work->timeout[3] = 0xe0;

	work->clock[0] = 0x07;
	work->clock[1] = 0x00;
	work->clock[2] = 0x00;
	work->clock[3] = 0x08;
	work->clock[4] = 0x74;
	work->clock[5] = 0x01;
	work->clock[6] = 0x00;
	work->clock[7] = 0x00;

	work->step[0] = 0x19;
	work->step[1] = 0x99;
	work->step[2] = 0x99;
	work->step[3] = 0x99;
}

void miner_gen_work(struct mm_work *mw, struct work *work)
{
	uint8_t merkle_root[32], merkle_sha[64];
	uint32_t *data32, *swap32, tmp32;
	int i;

	tmp32 = bswap_32(mw->nonce2);
	memcpy(mw->coinbase + mw->nonce2_offset, (uint8_t *)(&tmp32), sizeof(uint32_t));
	work->nonce2 = mw->nonce2++;

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

	debug32("Work nonce2:\n"); hexdump((uint8_t *)(&work->nonce2), 4);
	debug32("Generated header:\n"); hexdump(mw->header, 128);
	calc_midstate(mw, work);
	calc_prepare(mw, work);
}
