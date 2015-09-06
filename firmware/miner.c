/*
 * Author: Xiangfu Liu <xiangfu@openmobilefree.net>
 * Bitcoin:	1CanaaniJzgps8EV6Sfmpb7T8RutpaeyFn
 *
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */

#include <stdbool.h>
#include <stdint.h>

#include "minilibc/minilibc.h"
#include "system_config.h"
#include "defines.h"
#include "io.h"
#include "miner.h"
#include "sha256.h"
#include "twipwm.h"
#include "api.h"

// TODO: flip is useless if no nonce testing needed
static inline void flip(void *dest_p, const void *src_p, int count)
{
	uint32_t *dest = dest_p;
	const uint32_t *src = src_p;
	int i;

	for (i = 0; i < count / 4; i++)
		dest[i] = bswap_32(src[i]);
}

static void calc_midstate(struct mm_work *mw, struct work *work)
{
	int i;
	uint8_t *p = work->data;

	sha256_midstate(mw->header, p);

	p = (uint8_t *)(mw->header + 64);
	for (i = 0; i < 12; i++)
		*(work->data + 32 + i) = p[11 - i];
}

/* Total: 4W + 19W = 23W
 * TaskID_H:1, TASKID_L:1, STEP:1, TIMEOUT:1,
 * CLK_CFG:2, a2, Midsate:8, e0, e1, e2, a0, a1, Data:3
 */

void roll_work(struct work *work, int ntime_offset)
{
	uint32_t *work_ntime;
	uint32_t ntime;

	if (!ntime_offset)
		return;

	/* The block header: for test nonce  */
	work_ntime = (uint32_t *)(work->header + 68);
	*work_ntime += ntime_offset;

	/* The data: for ASIC api */
	work_ntime = (uint32_t *)(work->data + 36);
	ntime = bswap_32(*work_ntime);
	ntime += ntime_offset;
	*work_ntime = bswap_32(ntime);

	sha256_precalc(work->data, work);
}

void miner_gen_nonce2_work(struct mm_work *mw, uint32_t nonce2, struct work *work)
{
	uint8_t *merkle_root = mw->header + mw->merkle_offset;
	uint32_t tmp32;
	int i;
	int nonce2_offset_posthash;
	int coinbase_len_posthash;

	tmp32 = bswap_32(nonce2);
	work->nonce2 = nonce2;

	nonce2_offset_posthash = (mw->nonce2_offset % SHA256_BLOCK_SIZE) + 32;
	coinbase_len_posthash = mw->coinbase_len - mw->nonce2_offset + (mw->nonce2_offset % SHA256_BLOCK_SIZE);
	memcpy(mw->coinbase + nonce2_offset_posthash, (uint8_t *)(&tmp32), mw->nonce2_size);

	// TODO: Merge from here to END into dsha256_merkle
	dsha256_posthash(mw->coinbase, mw->coinbase_len, coinbase_len_posthash, merkle_root);

#ifdef DEBUG_STRATUM
	debug32("MR:\n");
	hexdump(merkle_root, 32);
	debug32("CB:\n");
	hexdump(mw->coinbase, mw->coinbase_len);
#endif

	for (i = 0; i < mw->nmerkles; i++) {
		dsha256_merkle(mw->header + mw->merkle_offset, mw->merkles[i], merkle_root);

#ifdef DEBUG_STRATUM
		debug32("MR[%d]: \n", i);
		hexdump(merkle_root, 32);
#endif

	}
	flip(merkle_root, merkle_root, 32);
	// END

#ifdef DEBUG_STRATUM
	debug32("MR:\n");
	hexdump(merkle_root, 32);
#endif

	// TODO: skip if no nonce testing needed.
	memcpy(work->header, mw->header, 128);

#ifdef DEBUG_STRATUM
	hexdump(mw->header, 128);
	hexdump(work->header, 128);
#endif

	calc_midstate(mw, work);

	sha256_precalc(work->data, work);
	work->memo = mw->job_id | mw->pool_no;
}

static int fulltest(const unsigned char *hash, const unsigned char *target)
{
	uint32_t *hash32 = (uint32_t *)hash;
	uint32_t *target32 = (uint32_t *)target;
	int rc = 1;
	int i;

	for (i = 28 / 4; i >= 0; i--) {
		uint32_t h32tmp = bswap_32(hash32[i]);
		uint32_t t32tmp = bswap_32(target32[i]);

		if (h32tmp > t32tmp) {
			rc = NONCE_VALID;
			break;
		}
		if (h32tmp < t32tmp) {
			rc = NONCE_DIFF;
			break;
		}
	}

	return rc;
}

int test_nonce(struct mm_work *mw, uint32_t nonce2, uint32_t nonce, int ntime_offset)
{
	nonce -= 0x4000;
#ifdef DEBUG_VERBOSE
	debug32("Test: %08x %08x %d\n", nonce2, nonce, ntime_offset);
#endif
	/* Generate the work base on nonce2 */
	struct work work;
	miner_gen_nonce2_work(mw, nonce2, &work);

	/* Roll work */
	roll_work(&work, ntime_offset);

	/* Write the nonce to block header */
	uint32_t *work_nonce = (uint32_t *)(work.header + 64 + 12);
	*work_nonce = bswap_32(nonce);

	/* Regen hash */
	uint32_t *data32 = (uint32_t *)(work.header);
	unsigned char swap[80];
	uint32_t *swap32 = (uint32_t *)swap;
	unsigned char hash1[32];
	uint32_t *hash_32 = (uint32_t *)(hash1 + 28);

	flip(swap32, data32, 80);
	dsha256(swap, 80, hash1);

	if (*hash_32 != 0)
		return NONCE_HW;

	/* Compare hash with target */
	return fulltest(hash1, mw->target);
}
