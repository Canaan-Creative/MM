#ifndef __SHA256_H__
#define __SHA256_H__

void SHA256(uint32_t output[8], const uint8_t *buf, uint64_t size); // internal API
void HASH256(uint8_t output[32], const uint8_t *buf, uint32_t size);

#endif
