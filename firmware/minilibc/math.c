/*
 * Author: Mikeqin <Fengling.Qin@gmail.com>
 *
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */
#include "minilibc.h"

#define LN10 2.30258
#define LNR 0.20024

float logf(float x)
{
	float result, y, delta;
	int k, h, i;

	/* FIXME: x <= 0 , INVALID */
	k = 0;
	h = 0;
	for (k = 0; x > 1; k++)
		x /= 10;

	for (; x <= 0.1; k -= 1)
		x *= 10;

	for (h = 0; x < 0.9047; h -= 1)
		x *= 1.2217;

	y = (x - 1) / (x + 1);
	result = 2 * y;
	delta = 2 * y;

	for (i = 1; delta >= 0.00000001; i += 2) {
		delta = delta * y * y * i / (i + 2);
		result += delta;
	}

	return k * LN10 + h * LNR + result;
}
