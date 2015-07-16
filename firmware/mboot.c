/*
 * Author: Mikeqin <Fengling.Qin@gmail.com>
 *
 * This is free and unencumbered software released into the public domain.
 * For details see the UNLICENSE file at the root of the source tree.
 */
#include <stdint.h>
#include <stddef.h>
#include "minilibc/minilibc.h"
#include "sdk/intr.h"
#include "system_config.h"
#include "defines.h"
#include "io.h"
#include "crc.h"
#include "protocol.h"
#include "mboot.h"

/* spi flash op code */
#define MBOOT_FLASH_PGPROG      (0x02)
#define MBOOT_FLASH_RDDATA      (0x03)
#define MBOOT_FLASH_RDSTATUS    (0x05)
#define MBOOT_FLASH_WEN         (0x06)
#define MBOOT_FLASH_SECERASE    (0x20)

/* spi flash address scope */
/* name             addr                  note
 * -----------------------------------------------------------
 * mcs0        0x00000  ~  mcs0 len    (mcs0 len must < 0x80000)
 * reserved1   mcs0 len ~  0x7ffff
 * mcs1        0x80000  ~  mcs1 len    (mcs1 len must < 0x40000)
 * reserved2   mcs1 len ~  0xfbfff
 * cfg         0xfc000  ~  0xfefff     (3 sectors)
 * reserved3   0xff000  ~  0xfff7f
 * mcs1 info   0xfff80  ~  0xfff90
 * reserved4   0xfff91  ~  0x100000
 */
#define MBOOT_FLASH_BASE          0
#define MBOOT_MCS1_ADDR_BASE      (MBOOT_FLASH_BASE + 0x80000)  /* compatiable */
#define MBOOT_MCS1_INFO_ADDR_BASE 0xfff80                       /* compatiable */
#define MBOOT_MCS1_INFO_LEN       16                            /* compatiable */
#define MBOOT_CFG_START           0xfc000
#define MBOOT_CFG_END             (MBOOT_CFG_START + (MBOOT_FLASH_SECUSED * MBOOT_FLASH_SECSIZE))

/* spi flash specs */
#define MBOOT_FLASH_PGSIZE      (256)
#define MBOOT_FLASH_SECSIZE     (16 * MBOOT_FLASH_PGSIZE)
#define MBOOT_FLASH_SECUSED     (3)

/* MBOOT start addr */
#define MBOOT_START_ADDR0       (0x0)
#define MBOOT_START_ADDR1       (0x0)

static struct lm32_rbt *rbt = (struct lm32_rbt *)RBT_BASE;
static struct lm32_mboot *mboot = (struct lm32_mboot*)MBOOT_BASE;
static uint8_t g_pgbuf[MBOOT_FLASH_PGSIZE];

static void mboot_disable(void)
{
	mboot->flash = LM32_MBOOT_CS | LM32_MBOOT_HOLD_N | LM32_MBOOT_WP_N; /* IDLE */
}

static void mboot_enable(void)
{
	mboot->flash = LM32_MBOOT_HOLD_N | LM32_MBOOT_WP_N;
}

static unsigned char mboot_bit(uint8_t bit)
{
	unsigned char tmp = 0;

	bit &= 1;
	mboot->flash = LM32_MBOOT_HOLD_N | LM32_MBOOT_WP_N | LM32_MBOOT_MOSI(bit);
	mboot->flash = LM32_MBOOT_SCL | LM32_MBOOT_HOLD_N | LM32_MBOOT_WP_N | LM32_MBOOT_MOSI(bit);
	tmp = mboot->flash;
	mboot->flash = LM32_MBOOT_HOLD_N | LM32_MBOOT_WP_N | LM32_MBOOT_MOSI(bit);
	tmp = LM32_MBOOT_MISO(tmp) & 1;
	return tmp;
}

static unsigned char mboot_byte(uint8_t byte)
{
	int8_t i;
	unsigned char miso = 0;

	for (i = 7; i >= 0; i--)
		miso = miso | (mboot_bit(byte >> i) << i);

	return miso;
}

static void mboot_spi_cmd(unsigned char op, unsigned int addr)
{
	mboot_enable();
	mboot_byte(op);
	mboot_byte((addr >> 16) & 0xff);
	mboot_byte((addr >> 8) & 0xff);
	mboot_byte(addr & 0xff);
}

static void mboot_spi(int byte_num, unsigned char *buf)
{
	int i;

	mboot_enable();
	for (i = 0; i < byte_num; i++) {
		buf[i] = mboot_byte(buf[i]);
	}
	mboot_disable();
}

static void mboot_spi_op(unsigned char op, int byte_num, unsigned int addr, unsigned char *buf)
{
	mboot_spi_cmd(op, addr);
	mboot_spi(byte_num, buf);
}

static void mboot_spi_write_enable(void)
{
	unsigned char buf[1];

	buf[0] = MBOOT_FLASH_WEN;
	mboot_spi(1, buf);
}

static void mboot_spi_wait_busy(void)
{
	unsigned char buf;

	mboot_enable();
	while (1) {
		mboot_byte(MBOOT_FLASH_RDSTATUS);
		buf = mboot_byte(0x00);
		if ((buf & 0x01) == 0x00)
			break;
	}
	mboot_disable();
}

static void icap_enable(void)
{
	mboot->icap_o = 0;
}

static void icap_disable(void)
{
	mboot->icap_o = LM32_ICAP_CLK | LM32_ICAP_CE | LM32_ICAP_WRITE;
}

static void icap_write_16bit(uint16_t data_o)
{
	mboot->icap_o = data_o;
	mboot->icap_o = LM32_ICAP_CLK | data_o;
	mboot->icap_o = data_o;
}

static int icap_mboot_start(unsigned int boot_addr0, unsigned int boot_addr1)
{
	delay(1000);
	debug32("icap sent mboot cmd\n");
	icap_enable();
	icap_write_16bit(0x2000);
	icap_write_16bit(0x2000);
	icap_write_16bit(0x2000);
	icap_write_16bit(0x2000);

	icap_write_16bit(0xffff);
	icap_write_16bit(0xaa99);
	icap_write_16bit(0x5566);
	icap_write_16bit(0x31e1);
	icap_write_16bit(0xffff);
	icap_write_16bit(0x3261);
	icap_write_16bit(boot_addr1 & 0xffff);//MultiBoot Start Address[15:0]
	icap_write_16bit(0x3281);
	icap_write_16bit(((boot_addr1 >> 16) & 0xff) | 0x0300);//Opcode and MultiBoot Start Address[23:16]
	icap_write_16bit(0x32a1);
	icap_write_16bit(boot_addr0 & 0xffff);//FallBack Start Address [15:0]
	icap_write_16bit(0x32c1);
	icap_write_16bit(((boot_addr0 >> 16) & 0xff) | 0x0300);//Opcode and Fallback Start Address [23:16]

	icap_write_16bit(0x32e1);
	icap_write_16bit(0x0000);

	icap_write_16bit(0x30a1);
	icap_write_16bit(0x0000);

	icap_write_16bit(0x3301);
	icap_write_16bit(0x2100);

	icap_write_16bit(0x3201);
	icap_write_16bit(0x001f);

	icap_write_16bit(0x30a1);
	icap_write_16bit(0x000e);

	icap_write_16bit(0x2000);
	icap_write_16bit(0x2000);
	icap_write_16bit(0x2000);
	icap_write_16bit(0x2000);

	icap_disable();
	delay(5000);
	return 0;
}

void mboot_spi_read(int byte_num, unsigned int addr, unsigned char *buf)
{
#ifdef DEBUG_VERBOSE
	debug32("D: R %x\n", addr);
#endif
	mboot_spi_op(MBOOT_FLASH_RDDATA, byte_num, addr, buf);
}

int mboot_spi_write(int byte_num, unsigned int addr, unsigned char *buf)
{
#ifdef DEBUG_VERBOSE
	debug32("D: W %x\n", addr);
#endif
	mboot_spi_write_enable();
	if (addr >= MBOOT_CFG_START && addr < MBOOT_CFG_END) {
		mboot_spi_op(MBOOT_FLASH_PGPROG, byte_num, addr, buf);
		mboot_spi_wait_busy();
	} else
		return 1;

	return 0;
}

int mboot_spi_erase_sector(unsigned int addr)
{
#ifdef DEBUG_VERBOSE
	debug32("D: E %x\n", addr);
#endif
	mboot_spi_write_enable();
	if (addr >= MBOOT_CFG_START && addr < MBOOT_CFG_END) {
		mboot_spi_op(MBOOT_FLASH_SECERASE, 0, addr, NULL);
		delay(100);
		mboot_spi_wait_busy();
	} else
		return 1;

	return 0;
}

/* For low code segment size, pagesize is enough for save the configration */
static int mboot_find_config(uint32_t *pos, uint8_t *pgbuf)
{
	uint8_t i;
	uint16_t crc, oldcrc;
	uint32_t cfgpos = MBOOT_CFG_START;

	for (i = 0; i < MBOOT_FLASH_SECUSED; i++) {
		memset(pgbuf, 0, MBOOT_FLASH_PGSIZE);
		mboot_spi_read(MBOOT_FLASH_PGSIZE, cfgpos, pgbuf);

		crc = crc16(pgbuf, sizeof(struct mm_config) - 2);
		oldcrc = ((pgbuf[sizeof(struct mm_config) - 2] << 8) |
				(pgbuf[sizeof(struct mm_config) - 1]));
#ifdef DEBUG_VERBOSE
		debug32("C1:%x,C2:%x\n", crc, oldcrc);
		debug32("H1:%x,H2:%x\n", pgbuf[0], pgbuf[1]);
#endif
		if ((pgbuf[0] == AVA4_H1) && (pgbuf[1] == AVA4_H2)
				&& (oldcrc == crc)) {
			*pos = cfgpos;
			return 0;
		} else {
			cfgpos += MBOOT_FLASH_SECSIZE;
			continue;
		}
	}

	if (i == MBOOT_FLASH_SECUSED) {
		debug32("D: MFC :(\n");
	}

	return 1;
}

int mboot_save_config(struct mm_config *config)
{
	uint32_t cfgpos;
	struct mm_config *pconfig = NULL;

	config->crc = crc16((unsigned char*)config, sizeof(struct mm_config) - 2);

	if (!mboot_find_config(&cfgpos, g_pgbuf)) {
		mboot_spi_erase_sector(cfgpos);

		if ((cfgpos + MBOOT_FLASH_SECSIZE) >= (MBOOT_CFG_START + MBOOT_FLASH_SECSIZE * MBOOT_FLASH_SECUSED))
			cfgpos = MBOOT_CFG_START;
		else
			cfgpos += MBOOT_FLASH_SECSIZE;
	} else
		cfgpos = MBOOT_CFG_START;

	debug32("MSC: %x\n", cfgpos);
	mboot_spi_erase_sector(cfgpos);
	memcpy(g_pgbuf, (unsigned char*)config, sizeof(struct mm_config));
	pconfig = (struct mm_config*)g_pgbuf;
	pconfig->crc = crc16(g_pgbuf, sizeof(struct mm_config) - 2);
	mboot_spi_write(sizeof(struct mm_config), cfgpos, g_pgbuf);

	return 0;
}

int mboot_load_config(struct mm_config *config)
{
	uint32_t cfgpos;

	if (!mboot_find_config(&cfgpos, g_pgbuf)) {
		memcpy(config, g_pgbuf, sizeof(struct mm_config));
		return 0;
	}

	return 1;
}

void mboot_reset_config(struct mm_config *config)
{
	uint8_t i;

	if (config) {
		config->header[0] = AVA4_H1;
		config->header[1] = AVA4_H2;
		config->temp = 0;
		for (i = 0; i < MINER_COUNT; i++) {
			config->vol_eco[i] = ASIC_VOL_ECO;
			config->vol_normal[i] = ASIC_VOL_NORMAL;
			config->vol_turbo[i] = ASIC_VOL_TURBO;
		}
		config->reserved = 0;
		config->crc = crc16((unsigned char*)config, sizeof(struct mm_config) - 2);
	}
}

void mboot_run_rbt(void)
{
	if (readl(&rbt->rbt) == 0x1)
		icap_mboot_start(MBOOT_START_ADDR0, MBOOT_START_ADDR1);
}

#ifdef DEBUG_VERBOSE
void mboot_flash_test(void)
{
#define MBOOT_FLASH_ADDR MBOOT_CFG_START
	uint8_t pgbuf_w[MBOOT_FLASH_PGSIZE];
	uint8_t pgbuf_r[MBOOT_FLASH_PGSIZE];

	debug32("D: read page\n");
	memset(pgbuf_r, 0, MBOOT_FLASH_PGSIZE);
	mboot_spi_read(MBOOT_FLASH_PGSIZE, MBOOT_FLASH_ADDR, pgbuf_r);
	debug32("D: ori data \n");
	hexdump(pgbuf_r, MBOOT_FLASH_PGSIZE);

	mboot_spi_erase_sector(MBOOT_FLASH_ADDR);
	memset(pgbuf_r, 0, MBOOT_FLASH_PGSIZE);
	mboot_spi_read(MBOOT_FLASH_PGSIZE, MBOOT_FLASH_ADDR, pgbuf_r);
	debug32("D: erase data \n");
	hexdump(pgbuf_r, MBOOT_FLASH_PGSIZE);

	memset(pgbuf_w, 0, MBOOT_FLASH_PGSIZE);
	pgbuf_w[0] = 0xaa;
	pgbuf_w[1] = 0x55;
	pgbuf_w[MBOOT_FLASH_PGSIZE - 2] = 0x55;
	pgbuf_w[MBOOT_FLASH_PGSIZE - 1] = 0xaa;
	mboot_spi_write(MBOOT_FLASH_PGSIZE, MBOOT_FLASH_ADDR, pgbuf_w);
	memset(pgbuf_r, 0, MBOOT_FLASH_PGSIZE);
	mboot_spi_read(MBOOT_FLASH_PGSIZE, MBOOT_FLASH_ADDR, pgbuf_r);
	debug32("D: new data \n");
	hexdump(pgbuf_r, MBOOT_FLASH_PGSIZE);
}
#endif

