/*
 * Author: Xiangfu Liu <xiangfu@openmobilefree.net>
 * Bitcoin:	1CanaaniJzgps8EV6Sfmpb7T8RutpaeyFn
 *
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */

#ifndef __MINER_H__
#define __MINER_H__

#include <stdbool.h>
#include <stdint.h>

struct work {
	uint32_t nonce2;
	uint8_t	task_id[8];	/* Nonce2 + job_id etc */
	uint8_t	step[4];
	uint8_t	timeout[4];
	uint8_t	clock[8];

	uint8_t a2[4];
	uint8_t e0[4];
	uint8_t e1[4];
	uint8_t e2[4];
	uint8_t a0[4];
	uint8_t a1[4];
	uint8_t data[44]; 	/* midstate[32] + data[12] */
};

struct result {
	uint8_t miner_id[4];      /* The miner ID */
	uint8_t	task_id[8];	 /* Same with work task_id */
	uint8_t	timeout[4];
	uint8_t nonce[4];
};

struct mm_work {
	uint8_t *jobid;

	size_t coinbase_len;
	uint8_t *coinbase;

	char *nonce1;
	uint32_t nonce2;
	int nonce2_offset;
	int nonce2_size; /* only 4 is support atm. */

	int merkle_offset;
	int nmerkles;
	uint8_t *merkles[10];

	uint8_t difficulty; /* number of leading zeros bits required
			     * (for a valid share) */
	bool rollntime; /* whether rollntime is accepted */
	bool clean;	/* flush all prior jobs (cut) */

	uint8_t *header;
};

void miner_init_work(struct mm_work *mw, struct work *work);
void miner_gen_work(struct mm_work *mw, struct work *work);

#endif /* __MINER_H__ */
