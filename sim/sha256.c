// simple and compact SHA256 implementation
#include <stdint.h>
#include <string.h>

/*
 * The literal definitions according to FIPS180-2 would be:
 *
 *      Ch(x, y, z)     (((x) & (y)) ^ ((~(x)) & (z)))
 *      Maj(x, y, z)    (((x) & (y)) | ((x) & (z)) | ((y) & (z)))
 *
 * We use logical equivalents which require one less op.
 */
#define Ch(x, y, z)     ((z) ^ ((x) & ((y) ^ (z))))
#define Maj(x, y, z)    (((x) & (y)) ^ ((z) & ((x) ^ (y))))
#define Rot32(x, s)     (((x) >> s) | ((x) << (32 - s)))
#define SIGMA0(x)       (Rot32(x, 2) ^ Rot32(x, 13) ^ Rot32(x, 22))
#define SIGMA1(x)       (Rot32(x, 6) ^ Rot32(x, 11) ^ Rot32(x, 25))
#define sigma0(x)       (Rot32(x, 7) ^ Rot32(x, 18) ^ ((x) >> 3))
#define sigma1(x)       (Rot32(x, 17) ^ Rot32(x, 19) ^ ((x) >> 10))

static const uint32_t SHA256_K[64] = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
        0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
        0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
        0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
        0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
        0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
        0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
        0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
        0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
        0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
        0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
        0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
        0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
};

static void
SHA256Transform(uint32_t *H, const uint8_t *cp) {
        uint32_t a, b, c, d, e, f, g, h, t, T1, T2, W[64];

        for (t = 0; t < 16; t++, cp += 4)
                W[t] = (cp[0] << 24) | (cp[1] << 16) | (cp[2] << 8) | cp[3];

        for (t = 16; t < 64; t++)
                W[t] = sigma1(W[t - 2]) + W[t - 7] + sigma0(W[t - 15]) + W[t - 16];

        a = H[0]; b = H[1]; c = H[2]; d = H[3];
        e = H[4]; f = H[5]; g = H[6]; h = H[7];

        for (t = 0; t < 64; t++) {
                T1 = h + SIGMA1(e) + Ch(e, f, g) + SHA256_K[t] + W[t];
                T2 = SIGMA0(a) + Maj(a, b, c);
                h = g; g = f; f = e; e = d + T1;
                d = c; c = b; b = a; a = T1 + T2;
        }

        H[0] += a; H[1] += b; H[2] += c; H[3] += d;
        H[4] += e; H[5] += f; H[6] += g; H[7] += h;
}

/*static*/ void
SHA256(uint32_t output[8], const uint8_t *buf, uint64_t size) {
        static const uint32_t H0[8] = {
		0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
		0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
	};
        uint8_t pad[128];
        int padsize = size & 63;
        int i;

	memcpy(output, H0, sizeof(H0));

        for (i = 0; i < size - padsize; i += 64)
                SHA256Transform(output, (uint8_t *)buf + i);

        for (i = 0; i < padsize; i++)
                pad[i] = ((uint8_t *)buf)[size - padsize + i];

        for (pad[padsize++] = 0x80; (padsize & 63) != 56; padsize++)
                pad[padsize] = 0;

        for (i = 0; i < 8; i++)
                pad[padsize++] = (size << 3) >> (56 - 8 * i);

        for (i = 0; i < padsize; i += 64)
                SHA256Transform(output, pad + i);
}

void
HASH256(uint8_t output[32], const uint8_t *buf, uint32_t size) {
	uint32_t H[8];
	uint8_t H2[32];
	int i;

	SHA256(H, buf, size);

	for (i = 0; i < 32; i++)
		H2[i] = 0xff & (H[i/4] >> ((3-i % 4) * 8));

	SHA256(H, H2, 32);

	for (i = 0; i < 32; i++)
		output[i] = 0xff & (H[i/4] >> ((3-i % 4) * 8));
}
