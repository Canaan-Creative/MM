/*
 * Author: Minux
 * Author: Xiangfu Liu <xiangfu@openmobilefree.net>
 * Bitcoin:	1CanaaniJzgps8EV6Sfmpb7T8RutpaeyFn
 *
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */

#include <stdbool.h>

#include "sdk.h"
#include "minilibc.h"
#include "jsmn.h"

#include "system_config.h"
#include "defines.h"

#include "io.h"
#include "serial.h"
#include "jsmn_extend.h"
#include "miner.h"

#include "hexdump.c"

#ifdef DEBUG
char printf_buf32[32];
#define debug32(...)	do {				\
		m_sprintf(printf_buf32, __VA_ARGS__);	\
		serial_puts(printf_buf32);		\
	} while(0)
#else
#define debug32(...)
#endif

struct stratum_work stratum_work;

static void delay(volatile uint32_t i)
{
	while (i--)
		;
}

static void error(uint8_t n)
{
	volatile uint32_t *gpio = (uint32_t *)GPIO_BASE;
	uint8_t i = 0;

	while (1) {
		delay(4000000);
		if (i++ %2)
			writel(0x00000000 | (n << 24), gpio);
		else
			writel(0x00000000, gpio);
	}
}

const char *stratum = "{"
	"\"params\": [\"1380763043 1101\", "
	"\"5506102164edbcbd90a8b0d617618e0c724dad2a58b640620000000000000000\", "
	"\"01000000010000000000000000000000000000000000000000000000000000000000000000ffffffff510301fd030d00456c696769757300524cc5a3fabe6d6d6eac9f79d09de75047cd6dad7e13bd3067d6f6406b443d4347c3527c549f89e10400000000000000002f7373312f00\", "
	"\"ffffffff2f0893b912000000001976a9145ad4df36c38579badb7113919d5664ff24d8c30b88ac16302d0c000000001976a91415b533475bb31e9c992e054b7e210194b0b382e688ac95af210c000000001976a9142f790c69fbf82ca1cc1cd08c7bcd9785bf285af088aca39fb60b000000001976a914512fe364168d51895cf5d12825eb8478a021bc8c88ac935d5a0a000000001976a9146ba1807121fdcb390db917b4ce84b0dc8e70883e88ac6609bd08000000001976a91485c9ce4366b559c0a1a65940586d33ce1ccdc31f88ac55fc9907000000001976a914f8159668b1e026c4390e39567c6805b3669643cf88ac24e22b06000000001976a914b259972b33d57f702b329a4d5a6b67a0759ed3ac88ac49bfb004000000001976a91480935fe3274069f6b0fd305e5c4d837991dbb12788ac46c31604000000001976a914e444fca561fcd4ecc12d75d145a8fdd0602199fe88acd7930003000000001976a9148c35417ab7ad8bab823bb4e8abf3b3b2c48a85b688acac48ef02000000001976a9145fbf44cc19f3cf91b94c17caff18b2f286813f7888ace3a96902000000001976a9148eafb1970e427d4d6ebb213b737b7dcdc96791e988ac5299ca01000000001976a914fe14cc64463da4f490198e4059ba7ad251d22e5b88ac2f37a601000000001976a9146f9f75a14ac7aa1c2b5f7c9aa9921ebc0bb84fea88acd4598d01000000001976a9146841e967d950fdd25fdd2f7a8ca87eb8f1d85ca788ace4308b01000000001976a914582b9290dfe30bb9b449de96043cb9db743e7a2288ac56c38a01000000001976a91497a8b3304d167fcef320136c67b44ae9ea97aefb88ac3b948801000000001976a9144f1cff8f980b15be09510a276c19c6aeb379381a88ac5cb27301000000001976a914ae8b96ace8e4e9325d304b962529e98a86334c9288acb2bc5d01000000001976a914af16a73fabf6df7446ff7bc9eabb11ad7d59382788ac3ab82101000000001976a914c8459142759347c481684590dabf11d9f91e143f88acddee1201000000001976a9140ab7c66bdfeada57e4619e896e17f754dabedcef88ac22741201000000001976a914d1a625cb20c6c513bb65b9b1e5699ba6e9b6c5b288ac0e5f1201000000001976a9140de2ca64176650db7763f4f3d6e362564b44fed588ac687c0d01000000001976a914fefa4447eb8f681b8473d43e7d845e55f3a35afa88acc53d0d01000000001976a914472427e6c7bbf78ad5ace308f94315b4d27c313e88ac8a920c01000000001976a91425e424d1ef97788b58aff2d069f28503aef163e488ac04730c01000000001976a914ef6d92b3a6dab1689c90a4d3bc9687e4c42f6b9d88ac261e0a01000000001976a91425c94f424c6e3466d78162da9b1a72616eddeba488ac256a0701000000001976a9146a37e007a500d470ba6623570ac87980e39ce98a88ac678c0301000000001976a914c17e44923f07ddd95b030cee63e94bea7a28550288ac69050301000000001976a914be86e5669d71f8f87faf000165bb3ad0288ec7d388acaa2b0201000000001976a914454dda256c717d10e819851ee3874d1ffd33bfd688ac62a17d00000000001976a914efbc2767dc16218e89e11eff19f88a3e3b3f0a1b88ac342e4100000000001976a9143adb8dc33166fe5f9a86b8ed6e8f745f0cce39e988ac8a202f00000000001976a914e44a4389826eb1bfa040f3ce18b8fb2a763f9e5488aca6192300000000001976a9142c3e7a35ec9f75084971ba15390ffef10cef69be88ac1fb42000000000001976a9144543f47812977801c563c7f548ddc298f1c9d7ad88ac5dae1c00000000001976a9148e2b31c1668133263fdd2d57571712810ee3bfe388acd7281b00000000001976a9145e2a5618487fc541479fa5b4ae13cf9f2986d6ff88acf8a11800000000001976a91402b82aa28674e1e80df69e5418b6232d7c57e5e388accfcb1000000000001976a9142d9f14d72a85974f1726dca16366706576c68e0088ac1a291000000000001976a91404e575a332c660249c504703f3e81f486294f99c88acc0121000000000001976a9144ee3a81efbc3526426ca46b699fb6641d6c39cf088ac00000010000000001976a91480563a02639dbb5eaa5be230f172b036ade336c388ac51060603000000001976a9145399c3093d31e4b0af4be1215d59b857b861ad5d88ac00000000\", "
	"["
	"\"4e1e966afd6c076743f60d908e8607c166e79490662ac73004c0bd7650af16cf\", "
	"\"5d9f256aa0dc3b46c18933050cc82aae0c01b1ba1b882b2ff378102945ea0953\", "
	"\"61d1f8f8cb041c7c77b7379b0f1df2ebc0b26467318fac692fa1a75a00a22181\", "
	"\"0bc44973f3f3d06222a5fef231d75e93757b6d1d54d189e4bf955fd35ef79a29\", "
	"\"c4a2bcf92d698bc3a5593bad9c8ea8bc7ca69a084ebfc304154ea1c0476832ea\", "
	"\"c20efa30fdfd8011553342bbdc10195a07e09d6e3e18547086908076ac00604c\", "
	"\"f95e1c938c0c93b0ee57d813d6648c2d95fe7ab69677f043d8328afeefae4319\", "
	"\"5c7aa100fbe903aa3ab350edd3e80673dd0da1bb094d0aa05dd0b32730ecf7aa\", "
	"\"530d7cc9bb46d199c9a41dc9f89929669c9634ba4dc3152d28dd88acd3b2bcdb\", "
	"\"fb8093eaf26bb92f72c5e83d46db751142b5f27b7bc14413ec6d66b18c82e028\""
	"], "
	"\"00000002\", "
	"\"191cdc20\", "
	"\"524cc5a3\", "
	"false"
	"], "
	"\"id\": null, "
	"\"method\": \"mining.notify\""
	"}";

const uint32_t sha256_in[16] = {
	0x61626380, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000018};

const uint32_t sha256_in2[32] = {
	0x61626364, 0x62636465, 0x63646566, 0x64656667,
	0x65666768, 0x66676869, 0x6768696A, 0x68696A6B,
	0x696A6B6C, 0x6A6B6C6D, 0x6B6C6D6E, 0x6C6D6E6F,
	0x6D6E6F70, 0x6E6F7071, 0x80000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x000001C0};

static void sha256_transform(uint32_t *state, const uint32_t *input, int count)
{
	struct lm32_sha256 *sha256 = (struct lm32_sha256 *)SHA256_BASE;

	int i;

	writel(LM32_SHA256_CMD_INIT, &sha256->cmd);
	for (i = 0; i < count; i++) {
		writel(input[i], &sha256->in);
		if (!((i + 1) % 16))
			while (!(readl(&sha256->cmd) & LM32_SHA256_CMD_DONE))
				;
	}
	for (i = 0; i < 8; i++)
		state[i] = readl(&sha256->out);
}

static int bin_value(unsigned char ch)
{
        if ('0' <= ch && ch <= '9')
                return ch - '0';
        else if ('a' <= ch && ch <= 'f')
                return ch - 'a' + 0x0A;
        else if ('A' <= ch && ch <= 'F')
                return ch - 'A' + 0x0A;
        else
                return -1;
}

bool hex2bin(unsigned char *p, const char *hexstr, size_t len)
{
	while (*hexstr && len) {
		int a, b;

		a = ((bin_value(hexstr[0])<<4) & 0xF0);
		b = ((bin_value(hexstr[1])   ) & 0x0F);

		if (a == -1 || b == -1) {
			serial_puts("E: hex2bin sscanf failed:");
			serial_putc(hexstr[0]);
			serial_putc(hexstr[1]);
			serial_putc('\n');
			return false;
		}

		*p = (unsigned char)(a | b);

		hexstr += 2;
		p++;
		len--;
	}

	if (len == 0)
		return true;
	return false;
}

static bool parse_notify(const char *js, jsmntok_t *tokens, struct stratum_work *sw)
{
	size_t cb1_len, cb2_len;

	sw->job_id = &tokens[3];
	sw->prev_hash = &tokens[4];
	sw->coinbase1 = &tokens[5];
	sw->coinbase2 = &tokens[6];
	sw->merkles = &tokens[7];
	sw->bbversion = &tokens[7 + tokens[7].size + 1];
	sw->nbit = &tokens[7 + tokens[7].size + 2];
	sw->ntime = &tokens[7 + tokens[7].size + 3];
	sw->clean = &tokens[7 + tokens[7].size + 4];

	cb1_len = (*sw->coinbase1).end - (*sw->coinbase1).start;
	cb2_len = (*sw->coinbase2).end - (*sw->coinbase2).start;
	debug32("I: cb1/2 len: %d, %d\n", cb1_len, cb2_len);


	return true;
}

int main(void) {
	uint32_t i, j, state[8];

	uart_init();
	serial_puts(MM_VERSION);

	/* Test sha256 core: 1 block data*/
	sha256_transform(state, sha256_in, 16);
	hexdump((uint8_t *)state, 32);

	/* Test sha256 core: 2 block data*/
	sha256_transform(state, sha256_in2, 32);
	hexdump((uint8_t *)state, 32);

	/* Decode stratum to struct stratum */
	jsmn_parser parser;
	jsmntok_t tokens[30];
	jsmnidx_t idx_m, idx_n;
	jsmnerr_t r;

	jsmn_init(&parser);
	r = jsmn_parse(&parser, stratum, tokens, 256);
	if (r != JSMN_SUCCESS) {
		debug32("E: %d\n", r);
		error(0xe);
	}

	for (i = 0; i < 30; i++) {
		debug32("I: [%d]%d, %d, %d, %d->", i,
			  tokens[i].type, tokens[i].start,
			  tokens[i].end, tokens[i].size);
		for (j = tokens[i].start; j < tokens[i].end; j++)
			serial_putc(stratum[j]);
		serial_putc('\n');
	}

	idx_m = jsmn_object_get(stratum, tokens, "method");
	if (idx_m == JSMNIDX_ERR) {
		/* FIXME: do something else */
		error(0xe);
	}
	idx_n = jsmn_object_get(stratum, tokens, "mining.notify");
	if (idx_n == JSMNIDX_ERR) {
		/* FIXME: do something else */
		error(0xe);
	}
	if (idx_m == idx_n - 1) {
		if (!parse_notify(stratum, tokens, &stratum_work))
			error(0xe);
	}

	unsigned char p[32];
	if (!hex2bin(p, stratum + tokens[17].start, 32))
		error(0xe);
	hexdump(p, 32);

	/* Code should be never reach here */
	error(0xf);
	return 0;
}

/* vim: set ts=4 sw=4 fdm=marker : */
