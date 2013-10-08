/*
 * Author: Xiangfu Liu <xiangfu@openmobilefree.net>
 * Bitcoin:	1CanaaniJzgps8EV6Sfmpb7T8RutpaeyFn
 *
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */

#ifndef __MINER_H__
#define __MINER_H__

char *workpadding = "000000800000000000000000000000000000000000000000000000000000000000000000000000000000000080020000";

struct stratum_work {
	jsmntok_t *jobid;
	jsmntok_t *job_id;
	jsmntok_t *prev_hash;
	jsmntok_t *bbversion;
	jsmntok_t *nbit;
	jsmntok_t *ntime;
	jsmntok_t *clean;

	jsmntok_t *coinbase1;
	jsmntok_t *coinbase2;
	jsmntok_t *merkles;

	/* This coinbase needs 4K? */
	uint8_t coinbase[2*1024];
	uint8_t header[300];


	size_t cb_len;
	size_t header_len;
	int merkles_count;
	double diff;
};

#endif /* __MINER_H__ */
