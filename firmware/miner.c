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

static inline void flip32(void *dest_p, const void *src_p)
{
	uint32_t *dest = dest_p;
	const uint32_t *src = src_p;
	int i;

	for (i = 0; i < 8; i++)
		dest[i] = bswap_32(src[i]);
}

static inline void flip64(void *dest_p, const uint8_t *src_p)
{
	uint32_t *dest = dest_p;
	int i;

	for (i = 0; i < 16; i++) {
		dest[i] = src_p[i * 4 + 0] | src_p[i * 4 + 1] << 8 |
			src_p[i * 4 + 2] << 16 | src_p[i * 4 + 3] << 24;
	}
}

static inline void flip80(void *dest_p, const void *src_p)
{
	uint32_t *dest = dest_p;
	const uint32_t *src = src_p;
	int i;

	for (i = 0; i < 20; i++)
		dest[i] = bswap_32(src[i]);
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
	if (g_asic_freq == value)
		return;

	g_asic_freq = value;
}

uint32_t get_asic_freq()
{
	return g_asic_freq;
}

extern uint32_t g_clock_conf_count;

void miner_init_work(struct mm_work *mw, struct work *work)
{
	int timeout;
	int asic_freq;

	memcpy(work->task_id, (uint8_t *)(&mw->pool_no), 4);
	memcpy(work->task_id + 4, (uint8_t *)(&work->nonce2), 4);

	work->clock[4] = 0x00;
	work->clock[5] = 0x00;
	work->clock[6] = 0x01;
	work->clock[7] = 0x74;

#if defined(AVALON2_A3255_MACHINE)
	timeout = 4294967 / (g_asic_freq * ASIC_COUNT); /* Time in ms */
	timeout *= CPU_FREQUENCY / 1000; /* Time in cpu clock */
	memcpy(work->timeout, &timeout, 4);

	asic_freq = g_asic_freq * 32 / 50 + 0x7FE0;
	work->clock[0] = (asic_freq & 0xff00) >> 8;
	work->clock[1] = asic_freq & 0xff;
	work->clock[2] = 0x00;
	work->clock[3] = (g_clock_conf_count >= 100) ? 0x1 : 0x7;

	work->step[0] = 0x24;
	work->step[1] = 0x92;
	work->step[2] = 0x49;
	work->step[3] = 0x25;

#elif defined(AVALON3_A3233_MACHINE) || defined(AVALON3_A3233_CARD)
	timeout = 4294967 / (g_asic_freq * (1024 / 65) * ASIC_COUNT); /* Time in ms */
	timeout -= 4;			 /* Some manual fix for A3233 */
	timeout *= CPU_FREQUENCY / 1000;     /* Time in cpu clock */
	memcpy(work->timeout, &timeout, 4);

	/* 768 / 65 * FREQ = HASHRATE */
	if (g_asic_freq == 200 ) asic_freq = 0xb3e00007;
	else if (g_asic_freq == 225 ) asic_freq = 0xb4600007;
	else if (g_asic_freq == 250 ) asic_freq = 0xb4e00007;
	else if (g_asic_freq == 275 ) asic_freq = 0xb5600007;
	else if (g_asic_freq == 300 ) asic_freq = 0xb5e00007;
	else if (g_asic_freq == 325 ) asic_freq = 0xb6600007;
	else if (g_asic_freq == 350 ) asic_freq = 0xb6e00007;
	else if (g_asic_freq == 375 ) asic_freq = 0xa3a00007;
	else if (g_asic_freq == 375 ) asic_freq = 0xb7600007;
	else if (g_asic_freq == 400 ) asic_freq = 0xa3e00007;
	else if (g_asic_freq == 425 ) asic_freq = 0xa4200007;
	else if (g_asic_freq == 450 ) asic_freq = 0xa4600007;
	else if (g_asic_freq == 475 ) asic_freq = 0xa4a00007;
	else if (g_asic_freq == 500 ) asic_freq = 0xa4e00007;
	else if (g_asic_freq == 506 ) asic_freq = 0x14e10007;
	else if (g_asic_freq == 518 ) asic_freq = 0x15010007;
	else if (g_asic_freq == 525 ) asic_freq = 0x12800007;
	else if (g_asic_freq == 550 ) asic_freq = 0x12a00007;
	else if (g_asic_freq == 575 ) asic_freq = 0xa5a00007;
	else if (g_asic_freq == 593 ) asic_freq = 0x15c10007;
	else if (g_asic_freq == 600 ) asic_freq = 0x15e10007;
	else if (g_asic_freq == 606 ) asic_freq = 0x15e10007;
	else if (g_asic_freq == 625 ) asic_freq = 0xa6200007;
	else if (g_asic_freq == 650 ) asic_freq = 0xa6600007;
	else if (g_asic_freq == 675 ) asic_freq = 0xa6a00007;
	else if (g_asic_freq == 700 ) asic_freq = 0xa6e00007;
	else if (g_asic_freq == 725 ) asic_freq = 0xa7200007;
	else if (g_asic_freq == 750 ) asic_freq = 0x93a00007;
	else if (g_asic_freq == 750 ) asic_freq = 0xa7600007;
	else if (g_asic_freq == 775 ) asic_freq = 0x93c00007;
	else if (g_asic_freq == 800 ) asic_freq = 0x93e00007;
	else if (g_asic_freq == 825 ) asic_freq = 0x94000007;
	else if (g_asic_freq == 850 ) asic_freq = 0x94200007;
	else if (g_asic_freq == 875 ) asic_freq = 0x94400007;
	else if (g_asic_freq == 900 ) asic_freq = 0x94600007;
	else if (g_asic_freq == 925 ) asic_freq = 0x94800007;
	else if (g_asic_freq == 950 ) asic_freq = 0x94a00007;
	else if (g_asic_freq == 975 ) asic_freq = 0x94c00007;
	else if (g_asic_freq == 1000) asic_freq = 0x94e00007;
	else if (g_asic_freq == 1025) asic_freq = 0x95000007;
	else if (g_asic_freq == 1050) asic_freq = 0x95200007;
	else if (g_asic_freq == 1075) asic_freq = 0x95400007;
	else if (g_asic_freq == 1100) asic_freq = 0x95600007;
	else if (g_asic_freq == 1125) asic_freq = 0x95800007;
	else if (g_asic_freq == 1150) asic_freq = 0x95a00007;
	else if (g_asic_freq == 1175) asic_freq = 0x95c00007;
	else if (g_asic_freq == 1200) asic_freq = 0x95e00007;
	else if (g_asic_freq == 1225) asic_freq = 0x96000007;
	else if (g_asic_freq == 1250) asic_freq = 0x96200007;
	else if (g_asic_freq == 1275) asic_freq = 0x96400007;
	else if (g_asic_freq == 1300) asic_freq = 0x96600007;
	else if (g_asic_freq == 1325) asic_freq = 0x96800007;
	else if (g_asic_freq == 1350) asic_freq = 0x96a00007;
	else if (g_asic_freq == 1375) asic_freq = 0x96c00007;
	else if (g_asic_freq == 1400) asic_freq = 0x96e00007;
	else if (g_asic_freq == 1425) asic_freq = 0x97000007;
	else if (g_asic_freq == 1450) asic_freq = 0x97200007;
	else if (g_asic_freq == 1475) asic_freq = 0x97400007;
	else if (g_asic_freq == 1500) asic_freq = 0x83a00007;
	else if (g_asic_freq == 1500) asic_freq = 0x97600007;
	else if (g_asic_freq == 1525) asic_freq = 0x83a00007;
	else if (g_asic_freq == 1550) asic_freq = 0x83c00007;
	else if (g_asic_freq == 1575) asic_freq = 0x83c00007;
	else if (g_asic_freq == 1600) asic_freq = 0x83e00007;
	else asic_freq = 0x00000001;

	work->clock[0] = ((asic_freq & 0xff000000) >> 24) & 0x7f;
	work->clock[1] = (asic_freq & 0x00ff0000) >> 16;
	work->clock[2] = (asic_freq & 0x0000ff00) >> 8;
	work->clock[3] = (g_clock_conf_count >= 100) ? 0x1 : 0x7;

	work->step[0] = 0x19;
	work->step[1] = 0x99;
	work->step[2] = 0x99;
	work->step[3] = 0x9a;	/* 2^32 / asic_count + 1:
				 * 5: 0x33333334
				 * 6: 0x2aaaaaab
				 * 7: 0x24924925
				 *10: 0x1999999a
				 */
#endif
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
static void calc_prepare(struct work *work, uint8_t *buf)
{
	uint32_t precalc[6];
	sha256_precalc(buf, buf + 32, 12, (uint8_t *)precalc);
	memcpy(work->a0, precalc + 0, 4);
	memcpy(work->a1, precalc + 1, 4);
	memcpy(work->a2, precalc + 2, 4);
	memcpy(work->e0, precalc + 3, 4);
	memcpy(work->e1, precalc + 4, 4);
	memcpy(work->e2, precalc + 5, 4);
}

void miner_gen_nonce2_work(struct mm_work *mw, uint32_t nonce2, struct work *work)
{
	uint8_t merkle_root[32], merkle_sha[64];
	uint8_t work_t[44];
	uint32_t *data32, *swap32, tmp32;
	int i;
	int nonce2_offset_posthash;
	int coinbase_len_posthash;

	tmp32 = bswap_32(nonce2);
	work->nonce2 = nonce2;

	if (mw->coinbase_len > AVA2_P_COINBASE_SIZE || !(mw->flags & AVA2_P_FULLCOINBASE)) {
		nonce2_offset_posthash = (mw->nonce2_offset % SHA256_BLOCK_SIZE) + 32;
		coinbase_len_posthash = mw->coinbase_len - mw->nonce2_offset + (mw->nonce2_offset % SHA256_BLOCK_SIZE);
		memcpy(mw->coinbase + nonce2_offset_posthash, (uint8_t *)(&tmp32), mw->nonce2_size);
		dsha256_posthash(mw->coinbase, mw->coinbase_len, coinbase_len_posthash, merkle_root);
	} else {
		memcpy(mw->coinbase + mw->nonce2_offset, (uint8_t *)(&tmp32), mw->nonce2_size);
		dsha256(mw->coinbase, mw->coinbase_len, merkle_root);
	}

	memcpy(merkle_sha, merkle_root, 32);

#ifdef DEBUG_STRATUM
	hexdump(merkle_root, 32);
	hexdump(mw->coinbase, mw->coinbase_len);
#endif

	for (i = 0; i < mw->nmerkles; i++) {
		memcpy(merkle_sha + 32, mw->merkles[i], 32);
		dsha256(merkle_sha, 64, merkle_root);
		memcpy(merkle_sha, merkle_root, 32);

#ifdef DEBUG_STRATUM
		debug32("DEBUG: M: \n");
		hexdump(merkle_root, 32);
#endif

	}
	data32 = (uint32_t *)merkle_sha;
	swap32 = (uint32_t *)merkle_root;
	flip32(swap32, data32);

#ifdef DEBUG_STRATUM
	debug32("Work: 3 hexdump\n");
	hexdump(merkle_root, 32);
#endif

	memcpy(mw->header + mw->merkle_offset, merkle_root, 32);
	memcpy(work->header, mw->header, 128);

#ifdef DEBUG_STRATUM
	hexdump(mw->header, 128);
	hexdump(work->header, 128);
#endif

	debug32("Work: nonce2 %08x\n", work->nonce2);
	calc_midstate(mw, work);

	memcpy(work_t, work->data, 44);
	rev(work_t, 32);
	rev(work_t + 32, 12);
	memcpy(work->data, work_t, 44);

	calc_prepare(work, work->data);
	memcpy((uint8_t *)(&tmp32), work->a1, 4);
	memcpy((uint8_t *)(&tmp32), work->a0, 4);
	memcpy((uint8_t *)(&tmp32), work->e2, 4);
	memcpy((uint8_t *)(&tmp32), work->e1, 4);
	memcpy((uint8_t *)(&tmp32), work->e0, 4);
	memcpy((uint8_t *)(&tmp32), work->a2, 4);
}

int fulltest(const unsigned char *hash, const unsigned char *target)
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

int test_nonce(struct mm_work *mw, struct result *ret)
{
	/* Decode nonce2 and nonce */
	uint32_t nonce2, nonce;
	memcpy((uint8_t *)(&nonce2), ret->task_id + 4, 4);
	memcpy((uint8_t *)(&nonce), ret->nonce, 4);

#ifdef AVALON2_A3255_MACHINE
	nonce -= 0x180;
#elif defined(AVALON3_A3233_MACHINE) || defined(AVALON3_A3233_CARD)
	nonce -= 0x1000;
#endif

	/* Generate the work base on nonce2 */
	struct work work;
	debug32("Test: %08x %08x\n", nonce2, nonce);
	miner_gen_nonce2_work(mw, nonce2, &work);

	/* Write the nonce to block header */
	uint32_t *work_nonce = (uint32_t *)(work.header + 64 + 12);
	*work_nonce = bswap_32(nonce);

	/* Regen hash */
	uint32_t *data32 = (uint32_t *)(work.header);
	unsigned char swap[80];
	uint32_t *swap32 = (uint32_t *)swap;
	unsigned char hash1[32];
	uint32_t *hash_32 = (uint32_t *)(hash1 + 28);

	flip80(swap32, data32);
	dsha256(swap, 80, hash1);

	if (*hash_32 != 0)
		return NONCE_HW;
	/* Compare hash with target */

	return fulltest(hash1, mw->target);
}
