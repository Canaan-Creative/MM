/*
 * Author: Xiangfu Liu <xiangfu@openmobilefree.net>
 * Bitcoin:	1CanaaniJzgps8EV6Sfmpb7T8RutpaeyFn
 *
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */

static struct lm32_sha256 *sha256 = (struct lm32_sha256 *)SHA256_BASE;

static void sha256_init()
{
	writel(LM32_SHA256_CMD_INIT, &sha256->cmd);
}

static void sha256_update(const uint32_t *input, unsigned int count)
{
	int i;

	for (i = 0; i < count; i++) {
		writel(input[i], &sha256->in);
		if (!((i + 1) % 16))
			while (!(readl(&sha256->cmd) & LM32_SHA256_CMD_DONE))
				;
	}
}

static void sha256_final(uint32_t *state)
{
	int i;

	for (i = 0; i < 8; i++)
		state[i] = readl(&sha256->out);
}

void sha256(uint32_t *state, const uint32_t *input, unsigned int count)
{
	sha256_init();
	sha256_update(input, count);
	sha256_final(state);
}
