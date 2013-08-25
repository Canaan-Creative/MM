#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include "sha256.h"

static void
dump(uint8_t *data, uint32_t size) {
	uint32_t i;
	const int byte_per_line = 32, byte_per_group = 4;
	for (i = 0; i < size; i++) {
		if (i % byte_per_line == 0) printf((i > 0 ? "\n%08x:" : "%08x:"), i);
		if (i % byte_per_group == 0) putchar(' ');
		printf("%02x", data[i]);
	}
	putchar('\n');
}

static uint8_t unhexify_digit(char input) {
	assert(isxdigit(input));

	input = toupper(input);

	if (input >= 'A') return input - 'A' + 10;
	else return input - '0';
}

static void
unhexify(uint8_t *output, const char *input) {
	assert(strlen(input) % 2 == 0);

	while (*input) {
		*output++ = unhexify_digit(*input) * 16 + unhexify_digit(*(input+1));
		input += 2;
	}
}

int
main(int argc, char **argv) {
#ifdef TEST
	uint8_t in[] = "hello";
	uint8_t hash[32];
	uint8_t exp[32]; 
	
	unhexify(exp, "9595c9df90075148eb06860365df33584b75bff782a510c6cd4883a419833d50");

	HASH256(hash, in, strlen(in));

	if (memcmp(exp, hash, 32) != 0) {
		printf("got:\n");
		dump(hash, 32);
		printf("exp:\n");
		dump(exp, 32);
		return 1;
	}

	return 0;
#endif
}
