/*
 * Author: Xiangfu Liu <xiangfu@openmobilefree.net>
 * Bitcoin:	1CanaaniJzgps8EV6Sfmpb7T8RutpaeyFn
 *
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */

#include "sdk.h"
#include "minilibc.h"
#include "jsmn_extend.h"

jsmnidx_t jsmn_object_get(const char *js, jsmntok_t *tokens, const char *key)
{
	uint32_t i = 1;
	while (tokens[i].start != 0 && tokens[i].end != 0) {
		if (tokens[i].type == 3 &&
		    !strncmp(key, js + tokens[i].start, tokens[i].end - tokens[i].start))
			return i;
		i++;
	}

	return JSMNIDX_ERR;
}
