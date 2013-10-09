/*
 * Author: Xiangfu Liu <xiangfu@openmobilefree.net>
 * Bitcoin:	1CanaaniJzgps8EV6Sfmpb7T8RutpaeyFn
 *
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */

#ifndef _SHA256_H_
#define _SHA256_H_

#include "sdk.h"

void sha256(uint32_t *state, const uint32_t *input, unsigned int count);

#endif	/* _SHA256_H_ */
