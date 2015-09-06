/*
 * Author: Xiangfu Liu <xiangfu@openmobilefree.net>
 * Bitcoin:	1CanaaniJzgps8EV6Sfmpb7T8RutpaeyFn
 *
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */

#ifndef _SHA256_H_
#define _SHA256_H_

#include <stdint.h>
#include "miner.h"

#define SHA256_DIGEST_SIZE (256 / 8)
#define SHA256_BLOCK_SIZE  (512 / 8)

void sha256(const uint8_t *input, unsigned int count, uint8_t *state);
void dsha256(const uint8_t *input, unsigned int count, uint8_t *state);
void dsha256_merkle(const uint8_t *input1, const uint8_t *input2, uint8_t *state);
void dsha256_posthash(const uint8_t *input, unsigned int count, unsigned int count_posthash,uint8_t *state);
void sha256_precalc(const uint8_t *input, struct work *work);
void sha256_midstate(const uint8_t *input, uint8_t *state);

#endif	/* _SHA256_H_ */
