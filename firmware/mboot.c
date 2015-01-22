#include <stdint.h>

#include "minilibc.h"
#include "system_config.h"
#include "defines.h"
#include "intr.h"
#include "io.h"

int mboot_disable(void);
int mboot_enable(void);
unsigned char mboot_bit(int bit);
unsigned char mboot_byte(unsigned char byte);
int mboot_spi(int byte_num, unsigned char *buf);
int mboot_spi_cmd(unsigned char op, unsigned int addr);
int fpga_boot_sel(void);
int mboot_spi_read(int byte_num, unsigned int addr, unsigned char *buf);
int mboot_spi_write(int byte_num, unsigned int addr, unsigned char *buf);
int mboot_spi_op(unsigned char op, int byte_num, unsigned int addr, unsigned char *buf);
int mboot_spi_wait_busy(void);
int mboot_spi_erase_64kB_Block(unsigned int addr, unsigned char *buf);
int mboot_spi_write_enable(void);
int mboot_spi_cp(unsigned int mboot_file0_addr_start, unsigned int mboot_file0_blocknum, unsigned int mboot_file1_addr_start);

static struct lm32_rbt *rbt = (struct lm32_rbt *)RBT_BASE;
volatile unsigned int *mboot_flash = (unsigned int *)0x80000800;

int mboot_disable(void){
	*mboot_flash = 0x0b;//idle
	return 0;
}

int mboot_enable(void){
	*mboot_flash = 0x03;
	return 0;
}

unsigned char mboot_bit(int bit){
	unsigned char tmp = 0;
	unsigned char bit_buf = 0;
	bit_buf = bit & 1;
	*mboot_flash = 0x03 | (bit_buf << 2);
	*mboot_flash = 0x13 | (bit_buf << 2);
	tmp = *mboot_flash;
	*mboot_flash = 0x03 | (bit_buf << 2);
	tmp = (tmp >> 5) & 1;
	return tmp;
}

unsigned char mboot_byte(unsigned char byte){
	int i;
	unsigned char miso = 0;
	for(i = 7; i >= 0; i--){
		miso = miso | (mboot_bit(byte >> i) << i);
	}
	return miso;
}

int mboot_spi_cmd(unsigned char op, unsigned int addr){
	mboot_enable();
	mboot_byte(op);
        mboot_byte((addr >> 16) & 0xff);
        mboot_byte((addr >> 8) & 0xff); 
        mboot_byte(addr & 0xff); 
	return 0;
}

int mboot_spi(int byte_num, unsigned char *buf){
	int i;
	mboot_enable();
	for(i = 0; i < byte_num; i++){
		buf[i] = mboot_byte(buf[i]);
	}
	mboot_disable();
	return 0;
}

int mboot_spi_op(unsigned char op, int byte_num, unsigned int addr, unsigned char *buf){
	mboot_spi_cmd(op, addr);
        mboot_spi(byte_num, buf);
	return 0;
}
int mboot_spi_read(int byte_num, unsigned int addr, unsigned char *buf){
	mboot_spi_op(0x03, byte_num, addr, buf);
	return 0;
}

int mboot_spi_write(int byte_num, unsigned int addr, unsigned char *buf){
	if(addr >= 0x040000 && addr < 0x080000){
		mboot_spi_op(0x02, byte_num, addr, buf);
		mboot_spi_wait_busy();
	} else
		return 1;
	return 0;
}

int mboot_spi_erase_64kB_Block(unsigned int addr, unsigned char *buf){
	if(addr >= 0x040000 && addr < 0x080000){
		mboot_spi_op(0xd8, 4, addr, buf);
		mboot_spi_wait_busy();
	} else
		return 1;
        return 0;
}

int mboot_spi_write_enable(void){
	unsigned char buf[1];
	buf[0] = 0x06;
        mboot_spi(1, buf);
	return 0;
}

int mboot_spi_wait_busy(void){
	unsigned char buf;
        mboot_enable();
	while(1){
                mboot_byte(0x05);
                buf = mboot_byte(0x00);
		if((buf & 0x01) == 0x00)
			break;
        }
        mboot_disable();
        return 0;	
}

#define SPI_FLASH_PAGE_SIZE 256 //Bytes
#define SPI_FLASH_BLOCK_SIZE 0x10000 //Bytes
#define SPI_FLASH_BLOCK_NUM  8 //Block count
#define SPI_FLASH_CMD_LEN 4 //Bytes
int mboot_spi_cp(unsigned int mboot_file0_addr_start, unsigned int mboot_file0_blocknum, unsigned int mboot_file1_addr_start){
	unsigned char buf[SPI_FLASH_CMD_LEN + SPI_FLASH_PAGE_SIZE];//pagesize 256Byte
	int i;

	//Erase Blocks
	for(i = 0; i < (mboot_file0_blocknum * SPI_FLASH_BLOCK_SIZE); i += SPI_FLASH_BLOCK_SIZE){
		mboot_spi_write_enable();
		mboot_spi_erase_64kB_Block(mboot_file1_addr_start + i, buf);
		debug32("erase block addr = %x\n", mboot_file1_addr_start + i);
	}

	for(i = 0; i < (mboot_file0_blocknum * SPI_FLASH_BLOCK_SIZE); i += SPI_FLASH_PAGE_SIZE){
		mboot_spi_write_enable();
		mboot_spi_read(SPI_FLASH_CMD_LEN + SPI_FLASH_PAGE_SIZE, mboot_file0_addr_start + i, buf);
		mboot_spi_write(SPI_FLASH_CMD_LEN + SPI_FLASH_PAGE_SIZE, mboot_file1_addr_start + i, buf);
	}

	return 0;
}

volatile unsigned int *icap_o = (unsigned int *)0x80000804;
volatile unsigned int *icap_i = (unsigned int *)0x80000808;

int icap_enable(void);
int icap_disenable(void);
int icap_write_16bit(unsigned int data_o);
int icap_mboot_start(unsigned int mboot_file0_addr_start, unsigned int mboot_file1_addr_start);
int run_rbt(void);
int mboot(void);
int icap_wait_busy(void);

int icap_enable(void){
	*icap_o = 0x00000;
	return 0;
}

int icap_disenable(void){
	*icap_o = 0x70000;
	return 0;
}

int icap_write_16bit(unsigned int data_o){
	*icap_o = 0x00000 | (data_o & 0xffff);
	*icap_o = 0x40000 | (data_o & 0xffff);
	*icap_o = 0x00000 | (data_o & 0xffff);
	return 0;
}

int icap_wait_busy(void){
	unsigned int tmp;
	while(1){
		tmp = (*icap_i >> 16) & 1;
		if(tmp == 0)
			return 0;
	} 
}

#define MCS1_INFO_ADDR_BASE 0xfff80
#define MCS1_INFO_LEN 16 //bytes
#define MCS1_ADDR_BASE 0x80000
#define SPI_FLASH_PAGE 256 //byte

unsigned short mboot_mcs1_crc(unsigned int MCS1_LEN);
unsigned short mboot_mcs1_verify(void);

unsigned short mboot_mcs1_crc(unsigned int MCS1_LEN){
	unsigned int read_addr = MCS1_ADDR_BASE;
	unsigned int step = MCS1_LEN, tmp;
	unsigned short crc_out = 0;;
	unsigned char buf[1];

	while(read_addr < MCS1_ADDR_BASE + MCS1_LEN){
		if(step >= SPI_FLASH_PAGE)
			tmp = SPI_FLASH_PAGE;
		else
			tmp = step;
		mboot_spi_cmd(0x03, read_addr);
		read_addr += SPI_FLASH_PAGE;

		for(int i = 0; i < tmp; i++){
			buf[0] = mboot_byte(buf[0]);
			crc_out = mboot_crc16(crc_out, buf, 1);
		}

		mboot_disable();
		if(step < SPI_FLASH_PAGE)
			break;

		step -= SPI_FLASH_PAGE;
	}
	return crc_out;
}

unsigned short mboot_mcs1_verify(void){
	unsigned char buf[SPI_FLASH_PAGE];
	unsigned short MCS1_CRC16, MCS1_CRC16_GET;
	unsigned int MCS1_LEN;
	mboot_spi_read(MCS1_INFO_LEN, MCS1_INFO_ADDR_BASE, buf);
	MCS1_CRC16 = (buf[4] << 8) | buf[5];
	MCS1_LEN = (buf[6] << 24) | (buf[7] << 16) | (buf[8] << 8) | buf[9];

	if(buf[0] == 0 && buf[1] == 1 && buf[2] == 2 && buf[3] == 3){
		MCS1_CRC16_GET = mboot_mcs1_crc(MCS1_LEN);
		//debug32("MCS1_LEN = %d, MCS1_CRC16 = 0x%x, MCS1_CRC16_GET = 0x%x", MCS1_LEN, MCS1_CRC16, MCS1_CRC16_GET);
		if(MCS1_CRC16_GET == MCS1_CRC16){
			debug32("This is File0, MCS1 Valid!");
			icap_mboot_start(0, MCS1_ADDR_BASE);
		} else {
			debug32("This is File0, MCS1 NOT Valid!");
			return 1;
		}
	}
	debug32("This is File0, MCS1 is Empty!");

	return 0;
}

int run_rbt(void){
	unsigned int tmp;
	tmp = readl(&rbt->rbt);
	if(tmp == 0x1){
		icap_mboot_start(0x0, 0x0);
	}
	return 0;
}

int icap_mboot_start(unsigned int mboot_file0_addr_start, unsigned int mboot_file1_addr_start){
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
	icap_write_16bit(mboot_file1_addr_start & 0xffff);//MultiBoot Start Address[15:0]
	icap_write_16bit(0x3281);
	icap_write_16bit(((mboot_file1_addr_start >> 16) & 0xff) | 0x0300);//Opcode and MultiBoot Start Address[23:16]
	icap_write_16bit(0x32a1);
	icap_write_16bit(mboot_file0_addr_start & 0xffff);//FallBack Start Address [15:0]
	icap_write_16bit(0x32c1);
	icap_write_16bit(((mboot_file0_addr_start >> 16) & 0xff) | 0x0300);//Opcode and Fallback Start Address [23:16]
	
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

	icap_disenable();
	delay(5000);
	return 0;
}

int mboot(void){
	int MCS_ID = 0;
	if(!MCS_ID)
		mboot_mcs1_verify();
	return MCS_ID;
}

