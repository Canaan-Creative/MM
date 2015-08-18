/*
 * @brief libfunctions head file
 *
 * @note
 *
 * @par
 */
#ifndef __LIB_FUNCTIONS_H_
#define __LIB_FUNCTIONS_H_

#define bswap_16(value)  \
    ((((value) & 0xff) << 8) | ((value) >> 8))

#define bswap_32(value) \
    (((uint32_t)bswap_16((uint16_t)((value) & 0xffff)) << 16) | \
     (uint32_t)bswap_16((uint16_t)((value) >> 16)))

#define be32toh(x) bswap_32(x)

void delay(unsigned int ms);
#endif /* __LIB_FUNCTIONS_H_ */

