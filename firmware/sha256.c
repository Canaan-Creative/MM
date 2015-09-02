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
	int i, j, len_rem;
	uint8_t tmp[4];
	uint32_t *tmp32 = (uint32_t *)tmp;
	uint32_t *p = (uint32_t *)input;

	len_rem = count % SHA256_BLOCK_SIZE;

	for (i = 0; i <= len_rem - 4; i += 4)
		lm_sha256->din = p[i / 4];

	for (j = i; j < len_rem; j++)
		tmp[j - i] = input[j];
	tmp[j - i] = 0x80;
	for (j = len_rem + 1; j < i + 4; j++)
		tmp[j - i] = 0;
	lm_sha256->din = *tmp32;

	i += 4;

	if ((SHA256_BLOCK_SIZE - 9) < len_rem) {
		for (; i < SHA256_BLOCK_SIZE; i += 4)
			lm_sha256->din = 0;
		while (!(readl(&lm_sha256->cmd) & LM32_SHA256_CMD_DONE));
		i = 0;
	}

	for (; i < SHA256_BLOCK_SIZE - 4; i += 4)
		lm_sha256->din = 0;
	lm_sha256->din = count << 3;

	while (!(readl(&lm_sha256->cmd) & LM32_SHA256_CMD_DONE));
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

void dsha256m(const uint8_t *input1, const uint8_t *input2, uint8_t *state)
{
	int i;
	uint32_t *p;

	sha256_init();

	p = (uint32_t *)input1;
	for (i = 0; i < 8; i++)
		lm_sha256->din = p[i];
	p = (uint32_t *)input2;
	for (i = 0; i < 8; i++)
		lm_sha256->din = p[i];

	while (!(readl(&lm_sha256->cmd) & LM32_SHA256_CMD_DONE));

	lm_sha256->din = 0x80000000;
	for (i = 0; i < 14; i++)
		lm_sha256->din = 0;
	lm_sha256->din = 64 << 3;
	while (!(readl(&lm_sha256->cmd) & LM32_SHA256_CMD_DONE));

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

void sha256_precalc(const uint8_t *input, struct work *work)
{
	int i;
	uint32_t *p = (uint32_t *)input;

	writel(LM32_SHA256_CMD_RST, &lm_sha256->cmd);
	for (i = 7; i >= 0; i--)
		lm_sha256->hi = p[i];
	writel(LM32_SHA256_CMD_INIT, &lm_sha256->cmd);

	lm_sha256->din = p[10];
	lm_sha256->din = p[9];
	lm_sha256->din = p[8];
	lm_sha256->din = 0x80000000;
	for (i = 0; i < 11; i++)
		lm_sha256->din = 0;
	lm_sha256->din = 12 << 3;
	while (!(readl(&lm_sha256->cmd) & LM32_SHA256_CMD_DONE));

	p = (uint32_t *)(work->a0);
	*p = readl(&lm_sha256->pre);
	p = (uint32_t *)(work->a1);
	*p = readl(&lm_sha256->pre);
	p = (uint32_t *)(work->a2);
	*p = readl(&lm_sha256->pre);
	p = (uint32_t *)(work->e0);
	*p = readl(&lm_sha256->pre);
	p = (uint32_t *)(work->e1);
	*p = readl(&lm_sha256->pre);
	p = (uint32_t *)(work->e2);
	*p = readl(&lm_sha256->pre);
}
