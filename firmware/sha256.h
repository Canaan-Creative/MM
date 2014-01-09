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

#define SHA256_DIGEST_SIZE (256 / 8)
#define SHA256_BLOCK_SIZE  (512 / 8)

void sha256_init();
void sha256_update(const uint8_t *input, unsigned int count);
void sha256_final(uint8_t *state);
void sha256(const uint8_t *input, unsigned int count, uint8_t *state);
void dsha256(const uint8_t *input, unsigned int count, uint8_t *state);
void sha256_precalc(const uint8_t *h, const uint8_t *input, unsigned int count, uint8_t *state);

#endif	/* _SHA256_H_ */
