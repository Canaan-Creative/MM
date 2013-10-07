/*
 * Author: Xiangfu Liu <xiangfu@openmobilefree.net>
 * Bitcoin:	1CanaaniJzgps8EV6Sfmpb7T8RutpaeyFn
 *
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */

#ifndef _JSMN_EXTEND_H
#define _JSMN_EXTEND_H

#include "jsmn.h"

typedef int jsmnidx_t;

#define JSMNIDX_ERR	(-1)

jsmnidx_t jsmn_object_get(const char *js, jsmntok_t *tokens, const char *key);

#endif
