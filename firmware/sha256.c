/*
 * Author: Xiangfu Liu <xiangfu@openmobilefree.net>
 * Bitcoin:	1CanaaniJzgps8EV6Sfmpb7T8RutpaeyFn
 *
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */

#include <stdint.h>

#include "minilibc/minilibc.h"
#include "system_config.h"
#include "io.h"
#include "sha256.h"

static struct lm32_sha256 *lm_sha256 = (struct lm32_sha256 *)SHA256_BASE;

void sha256_init(void)
{
	writel(LM32_SHA256_CMD_RST, &lm_sha256->cmd);
	writel(LM32_SHA256_CMD_INIT, &lm_sha256->cmd);
}

static void write_block(const uint8_t *block)
{
	int i;
	uint32_t *p = (uint32_t *)block;

	for (i = 0; i < 16; i++)
		lm_sha256->din = p[i];

	while (!(readl(&lm_sha256->cmd) & LM32_SHA256_CMD_DONE))
		;
}

static void sha256_padding(const uint8_t *input, unsigned int count)
{
	int i, len_rem, block_nb;
	uint8_t block[SHA256_BLOCK_SIZE], block1[SHA256_BLOCK_SIZE];

	memset(block, 0, ARRAY_SIZE(block));
	memset(block1, 0, ARRAY_SIZE(block));

	len_rem = count % SHA256_BLOCK_SIZE;
	block_nb = ((SHA256_BLOCK_SIZE - 9) < (len_rem % SHA256_BLOCK_SIZE));


	if (block_nb) {
		for (i = 0; i < len_rem; i++)
			block[i] = input[i];
		block[i] = 0x80;

		block1[60] = ((count*8) & 0xff000000) >> 24;
		block1[61] = ((count*8) & 0x00ff0000) >> 16;
		block1[62] = ((count*8) & 0x0000ff00) >> 8;
		block1[63] = ((count*8) & 0x000000ff);

		write_block(block);
		write_block(block1);
	} else {
		for (i = 0; i < len_rem; i++)
			block[i] = input[i];
		block[i] = 0x80;
		block[60] = ((count*8) & 0xff000000) >> 24;
		block[61] = ((count*8) & 0x00ff0000) >> 16;
		block[62] = ((count*8) & 0x0000ff00) >> 8;
		block[63] = ((count*8) & 0x000000ff);
		write_block(block);
	}
}

void sha256_update(const uint8_t *input, unsigned int count)
{
	int i, len_blocks;

	len_blocks = count / SHA256_BLOCK_SIZE;

	if (len_blocks != 0) {
		for (i = 0; i < len_blocks * SHA256_BLOCK_SIZE; i += SHA256_BLOCK_SIZE)
			write_block(input + i);
	}
}

static void sha256_precalc_final(uint8_t *state)
{
	int i;
	uint32_t *p = (uint32_t *)state;

	for (i = 0; i < 6 ; i++)
		p[i] = readl(&lm_sha256->pre);
}

void sha256_final(uint8_t *state)
{
	int i;
	uint32_t *p = (uint32_t *)state;

	for (i = 0; i < 8; i++)
		p[i] = readl(&lm_sha256->hash);
}

void sha256(const uint8_t *input, unsigned int count, uint8_t *state)
{
	sha256_init();
	sha256_update(input, count);
	sha256_padding(input + (count / SHA256_BLOCK_SIZE) * SHA256_BLOCK_SIZE, count);
	sha256_final(state);
}

static void sha256_double(void)
{
	writel(LM32_SHA256_CMD_DBL, &lm_sha256->cmd);
	while (!(readl(&lm_sha256->cmd) & LM32_SHA256_CMD_DONE))
		;
}

void dsha256(const uint8_t *input, unsigned int count, uint8_t *state)
{
	sha256_init();
	sha256_update(input, count);
	sha256_padding(input + (count / SHA256_BLOCK_SIZE) * SHA256_BLOCK_SIZE, count);
	sha256_double();
	sha256_final(state);
}

void dsha256_posthash(const uint8_t *input, unsigned int count, unsigned int count_posthash, uint8_t *state)
{
	int i;
	uint32_t *p = (uint32_t *)input;

	writel(LM32_SHA256_CMD_RST, &lm_sha256->cmd);
	for (i = 0; i < 8; i++)
		lm_sha256->hi = p[i];

	writel(LM32_SHA256_CMD_INIT, &lm_sha256->cmd);

	sha256_update(input + 32, count_posthash);
	sha256_padding(input + 32 + (count_posthash/ SHA256_BLOCK_SIZE) * SHA256_BLOCK_SIZE, count);
	sha256_init();
	sha256_double();
	sha256_final(state);
}

void sha256_precalc(const uint8_t *h, const uint8_t *input, unsigned int count, uint8_t *state)
{
	int i;
	uint8_t digest[12];
	uint32_t *p = (uint32_t *)h;

	writel(LM32_SHA256_CMD_RST, &lm_sha256->cmd);
	for (i = 7; i >= 0; i--)
		lm_sha256->hi = p[i];
	writel(LM32_SHA256_CMD_INIT, &lm_sha256->cmd);

	memcpy(digest + 0, input + 8, 4);
	memcpy(digest + 4, input + 4, 4);
	memcpy(digest + 8, input + 0, 4);

	sha256_update(digest, count);
	sha256_padding(digest + (count / SHA256_BLOCK_SIZE) * SHA256_BLOCK_SIZE, count);
	sha256_precalc_final(state);
}
