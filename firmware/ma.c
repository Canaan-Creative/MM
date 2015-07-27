/*
 * Author: Mikeqin <Fengling.Qin@gmail.com>
 *
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */
#include <string.h>
#include "ma.h"

void init_data(struct ma *ma_dat)
{
	memset(ma_dat->samples, 0, sizeof(uint8_t) * MA_COUNT);
	ma_dat->sample_cnt = 0;
	ma_dat->head = ma_dat->tail = 0;
	ma_dat->sum = 0;
}

void push_data(struct ma *ma_dat, bool data)
{
	if (ma_dat->sample_cnt < MA_COUNT)
		ma_dat->sample_cnt++;
	else {
		ma_dat->sum -= ma_dat->samples[ma_dat->tail];
		ma_dat->tail = (ma_dat->tail + 1) % MA_COUNT;
	}

	if (data) {
		ma_dat->samples[ma_dat->head] = 1;
		ma_dat->sum++;
	} else
		ma_dat->samples[ma_dat->head] = 0;

	ma_dat->head = (ma_dat->head + 1) % MA_COUNT;
}

