#include <stdio.h>
#include <stdint.h>

static inline uint8_t rev8(uint8_t d)
{
	int i;
	uint8_t out = 0;

	/* (from left to right) */
	for (i = 0; i < 8; i++)
		if (d & (1 << i))
			out |= (1 << (7 - i));

	return out;
}

/* http://www.onsemi.com/pub_link/Collateral/ADP3208D.PDF */
static inline uint32_t encode_voltage_adp3208d(uint32_t v)
{
	return rev8((0x78 - v / 125) << 1 | 1) << 8;
}

static inline uint32_t decode_voltage_adp3208d(uint32_t v)
{
	return (0x78 - (rev8(v >> 8) >> 1)) * 125;
}

/* http://www.onsemi.com/pub/Collateral/NCP5392P-D.PDF */
static inline uint32_t encode_voltage_ncp5392p(uint32_t v)
{
	if (v == 0)
		return 0xff00;
	else
		return rev8(((0x59 - (v - 5000) / 125) & 0xff) << 1 | 1) << 8;
}

static inline uint32_t decode_voltage_ncp5392p(uint32_t v)
{
	if (v == 0xff00)
		return 0;
	else
		return (0x59 - (rev8(v >> 8) >> 1)) * 125 + 5000;
}

int main()
{
	int v = 0, encode_val;
	int model = 0;

	printf("Input 0 for adp3208, 1 for ncp5392:");
	scanf("%d", &model);
	switch (model) {
		case 0:
			v = 0;
			while (v < 15000) {
				encode_val = encode_voltage_adp3208d(v);
				printf("%d --> %04x | %d --> %d\n", v, encode_val, encode_val, decode_voltage_adp3208d(encode_val));
				v += 125;
			}
			break;
		case 1:
			v = 5000;
			while (v <= 16000) {
				encode_val = encode_voltage_ncp5392p(v);
				printf("%d --> %04x | %d --> %d\n", v, encode_val, encode_val, decode_voltage_ncp5392p(encode_val));
				v += 125;
			}

			v = 0;
			encode_val = encode_voltage_ncp5392p(v);
			printf("%d --> %04x | %d --> %d\n", v, encode_val, encode_val, decode_voltage_ncp5392p(encode_val));
			break;
		default:
			printf("You select nothing\n");
			break;
	}
	return 0;
}
