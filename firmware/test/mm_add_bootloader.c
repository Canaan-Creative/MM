#include <stdio.h>
#include <stdlib.h>

unsigned char block_addr[8][17] =
{
":020000040008F2\n",
":020000040009F1\n",
":02000004000AF0\n",
":02000004000BEF\n",
":02000004000CEE\n",
":02000004000DED\n",
":02000004000EEC\n",
":02000004000FEB\n"
};
unsigned char mcs1_info[21];
unsigned char mcs1_info_ch[44];
unsigned short crc_init = 0;
unsigned int gmcs1_len;

void put_info(FILE *fp)
{
	int i;
	unsigned char tmp;

	fputc(':', fp);
	for (i = 0; i < 21; i++)
		fprintf(fp, "%02x", mcs1_info[i]);
	fputc('\n', fp);
}

unsigned int crc16_table[256] =
{
        0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
        0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
        0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
        0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
        0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
        0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
        0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
        0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
        0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
        0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
        0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
        0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
        0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
        0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
        0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
        0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
        0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
        0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
        0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
        0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
        0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
        0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
        0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
        0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
        0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
        0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
        0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
        0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
        0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
        0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
        0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
        0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};

int file_conect(void)
{
	FILE *fp_bootloader;
	FILE *fp_mm;
	FILE *fp_mmout;
	unsigned char tmp[64];
	int i, j, block_cnt = 0, flg = 0;

	fp_bootloader = fopen("./bootloader.mcs", "rt");
	if (fp_bootloader == NULL) {
		printf("bootloader.mcs open fail!\n");
		exit(1);
	}

	fp_mm = fopen("./mm.mcs", "rt");
	if (fp_mm == NULL) {
		printf("mm.mcs open fail!\n");
		exit(1);
	}

	fp_mmout = fopen("./mmout.mcs", "wb");
	if (fp_mmout == NULL) {
		printf("mmout.mcs open fail!\n");
		exit(1);
	}

	while (1) {
		for (i = 0; i < 64; i++) {
			tmp[i] = fgetc(fp_bootloader);
			if (tmp[i] == '\n')
				break;
			else if (i == 8 && tmp[0] == ':' && tmp[7] == '0' && tmp[8] == '1') {
				flg = 1;
				break;
			}
		}
		if (flg)
			break;
		else {
			for (j = 0; j <= i; j++)
				fputc(tmp[j], fp_mmout);
		}
	}
	printf("bootloader.mcs load to mmout.mcs done!\n");

	while (1) {
                for (i = 0; i < 64; i++) {
                        tmp[i] = fgetc(fp_mm);
                        if (tmp[i] == '\n')
                                break;
                }

		if (tmp[0] == ':' && tmp[7] == '0' && tmp[8] == '4') {
			for (j = 0; j < 16; j++)
				fputc(block_addr[block_cnt][j], fp_mmout);
			block_cnt++;
		} else {
			if (tmp[0] == ':' && tmp[7] == '0' && tmp[8] == '1')
				put_info(fp_mmout);
			for (j = 0; j <= i; j++)
                                fputc(tmp[j], fp_mmout);
		}

		if (tmp[0] == ':' && tmp[7] == '0' && tmp[8] == '1')
			break;
	}

	fclose(fp_bootloader);
	fclose(fp_mm);
	fclose(fp_mmout);

        return 0;
}


static char char2byte(char char0, char char1)
{
        if (char0 >= 'A' && char0 <= 'F')
                char0 = char0 - 'A' + 10;
        else
                char0 = char0 - '0';

        if (char1 >= 'A' && char1 <= 'F')
                char1 = char1 - 'A' + 10;
        else
                char1 = char1 - '0';

        return (char0 << 4) | char1;
}

void gen_mm_new()
{
        int i, byte_num;
        unsigned char tmp[1000];
        unsigned char data;
        FILE *mboot_mcs_fp;
        FILE *mboot_mcs_fp_new;

        gmcs1_len = 0;
        mboot_mcs_fp = fopen("./mm.mcs", "rt");
        if (mboot_mcs_fp == NULL) {
            printf("open mm.mcs error!\n");
            exit(1);
        }

        mboot_mcs_fp_new = fopen("./mm_new.mcs", "wb");
        if (mboot_mcs_fp_new == NULL) {
            printf("open mm_new.mcs error!\n");
            exit(1);
        }

        while (1) {
                i = 0;
                while (1) {
                        tmp[i] = fgetc(mboot_mcs_fp);
                        if (tmp[i] == '\n')
                                break;
			else if(i == 10 && tmp[0] == ':' && tmp[7] == '0' && tmp[8] == '1' &&
				tmp[9] == 'F' && tmp[10] == 'F')//:00000001FF
                                break;
                        i++;
                }

		if (tmp[7] == '0' && tmp[8] == '0') {
		        byte_num = char2byte(tmp[1], tmp[2]);
		        for (i = 0; i < byte_num * 2; i += 2) {
		                data = char2byte(tmp[9 + i], tmp[9 + i + 1]);
		                fputc(data, mboot_mcs_fp_new);
		                gmcs1_len++;
		        }
		} else if (tmp[7] == '0' && tmp[8] == '1')
		        break;
        }

        fclose(mboot_mcs_fp);
        fclose(mboot_mcs_fp_new);
}

unsigned short mboot_crc16(unsigned short crc_init, unsigned char *buffer, int len)
{
        unsigned short crc;

        crc = crc_init;
        while(len-- > 0)
            crc = crc16_table[((crc >> 8) ^ (*buffer++)) & 0xFF] ^ (crc << 8);

        return crc;
}

static void flash_prog_info(unsigned short crc, unsigned int len)
{
	int i;
	unsigned char tmp = 0;
	mcs1_info[0] = 0x10;
	mcs1_info[1] = 0xFF;
	mcs1_info[2] = 0x80;
	mcs1_info[3] = 0x00;

        mcs1_info[4] = 0x00;
        mcs1_info[5] = 0x01;
        mcs1_info[6] = 0x02;
        mcs1_info[7] = 0x03;

        mcs1_info[8] = crc >> 8;
        mcs1_info[9] = crc;

        mcs1_info[10] = 0x00;
        mcs1_info[11] = ((len >> 16) & 0xff);
        mcs1_info[12] = ((len >> 8) & 0xff);
        mcs1_info[13] =  (len & 0xff);

        mcs1_info[14] = 0x00;
        mcs1_info[15] = 0x00;
        mcs1_info[16] = 0x00;
        mcs1_info[17] = 0x00;

        mcs1_info[18] = 0x00;
        mcs1_info[19] = 0x00;

	for(i = 0; i <= 19; i++)
		tmp = tmp + mcs1_info[i];
	tmp = ~tmp;
	tmp = tmp + 1;
        mcs1_info[20] = tmp;

	printf("MCS1 INFO: ");
	for(i = 0; i < 21; i++){
		printf("%02x", mcs1_info[i]);
	}
	printf("\n");
}



void gen_mm_new_crc()
{
	FILE *mboot_mcs_fp_new;
	unsigned char FLASH_PAGE[1];
	int i;

	mboot_mcs_fp_new = fopen("./mm_new.mcs", "rt");
	for (i = 0; i < gmcs1_len; i++) {
		FLASH_PAGE[0] = fgetc(mboot_mcs_fp_new);
		crc_init = mboot_crc16(crc_init, &FLASH_PAGE[0], 1);
	}
	flash_prog_info(crc_init, gmcs1_len);
	fclose(mboot_mcs_fp_new);
}

void main(void)
{
	gen_mm_new();
	gen_mm_new_crc();
	file_conect();
}

