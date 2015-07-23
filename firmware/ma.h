/*
 * Author: Mikeqin <Fengling.Qin@gmail.com>
 *
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */
#ifndef _MA_H_
#define _MA_H_

#include <stdint.h>
#include <stdbool.h>

#define MA_COUNT	50

struct ma {
	uint8_t samples[MA_COUNT];
	uint8_t sample_cnt;
	uint8_t head, tail;
	uint8_t sum;
};

void init_data(struct ma *ma_dat);
void push_data(struct ma *ma_dat, bool data);

#endif /* _MA_H_ */

