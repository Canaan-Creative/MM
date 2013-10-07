/*
 * Author: Xiangfu Liu <xiangfu@openmobilefree.net>
 * Bitcoin:	1CanaaniJzgps8EV6Sfmpb7T8RutpaeyFn
 *
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */

#ifndef __MINER_H__
#define __MINER_H__

struct stratum_work {
	jsmntok_t *jobid;
	jsmntok_t *job_id;
	jsmntok_t *prev_hash;
	jsmntok_t *bbversion;
	jsmntok_t *nbit;
	jsmntok_t *ntime;

	size_t cb_len;
	size_t header_len;
	int merkles;
	double diff;
};

#endif /* __MINER_H__ */
