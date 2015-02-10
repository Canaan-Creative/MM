/*
 * Author: Mikeqin <Fengling.Qin@gmail.com>
 *
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */

#ifndef __MBOOT_H__
#define __MBOOT_H__

/* must 4bytes align, don't use	__attribute__(packed) */
struct mm_config {
	uint8_t header[2];
	int16_t temp;
	uint32_t vol_eco[MINER_COUNT];
	uint32_t vol_normal[MINER_COUNT];
	uint32_t vol_turbo[MINER_COUNT];
	int16_t reserved;
	uint16_t crc;
};

void mboot_spi_read(int byte_num, unsigned int addr, unsigned char *buf);
int mboot_spi_write(int byte_num, unsigned int addr, unsigned char *buf);
int mboot_spi_erase_sector(unsigned int addr);
int mboot_save_config(struct mm_config *config);
int mboot_load_config(struct mm_config *config);
void mboot_reset_config(struct mm_config *config);
void mboot_run_rbt(void);
#ifdef DEBUG_VERBOSE
void mboot_flash_test(void);
#endif
#endif /* __MBOOT_H__ */
