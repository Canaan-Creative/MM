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
#include "sdk.h"

char *blank_merkel = "0000000000000000000000000000000000000000000000000000000000000000";
char *workpadding = "000000800000000000000000000000000000000000000000000000000000000000000000000000000000000080020000";

struct work {
	unsigned char	data[128];
	unsigned char	midstate[32];
	unsigned char	target[32];
	unsigned char	hash[32];


	bool		stratum;
	char 		*job_id;
	uint32_t	nonce2;
	size_t		nonce2_len;
	char		*ntime;
	double		sdiff;
	char		*nonce1;
};

struct mm_work {
	uint8_t *jobid;

	size_t coinbase_len;
	uint8_t *coinbase;
	int cb_nonce2_offset;
	int cb_nonce2_size; /* only 4 is support atm. */
	int nbranches;
	uint8_t *merkels[10];
	uint8_t difficulty; /* number of leading zeros bits required
			     * (for a valid share) */
	bool rollntime; /* whether rollntime is accepted */
	bool clean;	/* flush all prior jobs (cut) */

	uint8_t *header;
};

#endif /* __MINER_H__ */
