#include "io.h"
static int test_data[16][18] = {
	{0x1bed3ba0,
	 0xa2cb45c1,
	 0xd8f8ef67,
	 0x12146495,
	 0xc44192c0,
	 0x7145fd6d,
	 0x974bf4bb,
	 0x8f41371d,
	 0x65c90d1e,
	 0x9cb18a17,
	 0xfa77fe7d,
	 0x13cdfd7b,
	 0x00639107,
	 0x62a5f25c,
	 0x06b168ae,
	 0x087e051a,
	 0x89517050,
	 0x4ac1d001},

	{0xc1680161,
	 0x9d8d4242,
	 0xe06e5fab,
	 0x25a54bbe,
	 0x222e8b87,
	 0x7848c34b,
	 0xeea79cd6,
	 0x528caf7e,
	 0x33cde02a,
	 0x983dab15,
	 0x8119ce2a,
	 0x1c9fc4ed,
	 0xdac8ce29,
	 0x6d0fd9da,
	 0x6e18f645,
	 0x087e051a,
	 0x2d547050,
	 0xe8dc86b1},

	{0x84a9e522,
	 0xb6327d35,
	 0x2a54a40c,
	 0x879a9334,
	 0xd962d729,
	 0x5600f2af,
	 0xf81e2a4b,
	 0x80a27591,
	 0x4064bca0,
	 0x612fc425,
	 0xa1d03421,
	 0xf8941ad5,
	 0xc5e7acf1,
	 0xb0496c6a,
	 0xc35604f4,
	 0x087e051a,
	 0x3c537050,
	 0x826cce7a},

	{0x1c11ef93,
	 0xb3baed4e,
	 0x2a54a40c,
	 0x879a9334,
	 0xd962d729,
	 0x5600f2af,
	 0xf81e2a4b,
	 0x80a27591,
	 0x4064bca0,
	 0x612fc425,
	 0xa1d03421,
	 0xfa941ad5,
	 0xc3efecf2,
	 0xb0496c6a,
	 0xc55604f4,
	 0x087e051a,
	 0x3e537050,
	 0x826cce7a},

	{0xad258354,
	 0xcb517d22,
	 0x81c9ffff,
	 0xdd3da630,
	 0xea65fc9d,
	 0x857ccb58,
	 0x8a776518,
	 0x67eb9370,
	 0xed14a977,
	 0xf1d2de33,
	 0xa1235784,
	 0x65d344fe,
	 0xc6e2feca,
	 0x431284bc,
	 0x5e2e867c,
	 0x087e051a,
	 0x70527050,
	 0xc290e5a7},

	{0xdaa34ad5,
	 0xb171b59c,
	 0xd8f8ef67,
	 0x12146495,
	 0xc44192c0,
	 0x7145fd6d,
	 0x974bf4bb,
	 0x8f41371d,
	 0x65c90d1e,
	 0x9cb18a17,
	 0xfa77fe7d,
	 0x19cdfd7b,
	 0x068a510a,
	 0x62a5f25c,
	 0x0cb168ae,
	 0x087e051a,
	 0x8f517050,
	 0x4ac1d001},

	{0x010dceb6,
	 0x220f1dbd,
	 0xd8f8ef67,
	 0x12146495,
	 0xc44192c0,
	 0x7145fd6d,
	 0x974bf4bb,
	 0x8f41371d,
	 0x65c90d1e,
	 0x9cb18a17,
	 0xfa77fe7d,
	 0x12cdfd7b,
	 0x81677107,
	 0x62a5f25c,
	 0x05b168ae,
	 0x087e051a,
	 0x88517050,
	 0x4ac1d001},

	{0x2bc36997,
	 0xb1231d47,
	 0x2a54a40c,
	 0x879a9334,
	 0xd962d729,
	 0x5600f2af,
	 0xf81e2a4b,
	 0x80a27591,
	 0x4064bca0,
	 0x612fc425,
	 0xa1d03421,
	 0xfc941ad5,
	 0xc1d82cf3,
	 0xb0496c6a,
	 0xc75604f4,
	 0x087e051a,
	 0x40537050,
	 0x826cce7a},

	{0x62af5f08,
	 0x312bf034,
	 0x740d56be,
	 0xe2e8080f,
	 0x19fae6f7,
	 0xc35ad8db,
	 0x42727a8e,
	 0xff62389c,
	 0xc572d901,
	 0xcc39201c,
	 0x92f7791a,
	 0x24d6e2cd,
	 0x67d1e0c0,
	 0xf55f91a6,
	 0x39171075,
	 0x087e051a,
	 0x13557050,
	 0x9ed94986},

	{0x2068a649,
	 0xb0cc4d3c,
	 0x2a54a40c,
	 0x879a9334,
	 0xd962d729,
	 0x5600f2af,
	 0xf81e2a4b,
	 0x80a27591,
	 0x4064bca0,
	 0x612fc425,
	 0xa1d03421,
	 0xf6941ad5,
	 0xc0016cf0,
	 0xb0496c6a,
	 0xc15604f4,
	 0x087e051a,
	 0x3a537050,
	 0x826cce7a},

	{0xc95c48ca,
	 0x26fe7d8b,
	 0xd8f8ef67,
	 0x12146495,
	 0xc44192c0,
	 0x7145fd6d,
	 0x974bf4bb,
	 0x8f41371d,
	 0x65c90d1e,
	 0x9cb18a17,
	 0xfa77fe7d,
	 0x16cdfd7b,
	 0x8156f105,
	 0x62a5f25c,
	 0x09b168ae,
	 0x087e051a,
	 0x8c517050,
	 0x4ac1d001},

	{0x1033995b,
	 0x9a7ce254,
	 0xe06e5fab,
	 0x25a54bbe,
	 0x222e8b87,
	 0x7848c34b,
	 0xeea79cd6,
	 0x528caf7e,
	 0x33cde02a,
	 0x983dab15,
	 0x8119ce2a,
	 0x189fc4ed,
	 0xdab84e2b,
	 0x6d0fd9da,
	 0x6a18f645,
	 0x087e051a,
	 0x29547050,
	 0xe8dc86b1},

	{0x93753bfc,
	 0x2a6249eb,
	 0xe06e5fab,
	 0x25a54bbe,
	 0x222e8b87,
	 0x7848c34b,
	 0xeea79cd6,
	 0x528caf7e,
	 0x33cde02a,
	 0x983dab15,
	 0x8119ce2a,
	 0x239fc4ed,
	 0x545dae3e,
	 0x6d0fd9da,
	 0x7518f645,
	 0x087e051a,
	 0x34547050,
	 0xe8dc86b1},

	{0x9ea1e81d,
	 0xa82e81f6,
	 0xe06e5fab,
	 0x25a54bbe,
	 0x222e8b87,
	 0x7848c34b,
	 0xeea79cd6,
	 0x528caf7e,
	 0x33cde02a,
	 0x983dab15,
	 0x8119ce2a,
	 0x249fc4ed,
	 0xd369ce3d,
	 0x6d0fd9da,
	 0x7618f645,
	 0x087e051a,
	 0x35547050,
	 0xe8dc86b1},

	{0x873c2a5e,
	 0x2d1a8543,
	 0x81c9ffff,
	 0xdd3da630,
	 0xea65fc9d,
	 0x857ccb58,
	 0x8a776518,
	 0x67eb9370,
	 0xed14a977,
	 0xf1d2de33,
	 0xa1235784,
	 0x5ad344fe,
	 0x29ebded7,
	 0x431284bc,
	 0x532e867c,
	 0x087e051a,
	 0x65527050,
	 0xc290e5a7},

	{0xa2bef63f,
	 0x30101538,
	 0x2a54a40c,
	 0x879a9334,
	 0xd962d729,
	 0x5600f2af,
	 0xf81e2a4b,
	 0x80a27591,
	 0x4064bca0,
	 0x612fc425,
	 0xa1d03421,
	 0xf7941ad5,
	 0x41054cf0,
	 0xb0496c6a,
	 0xc25604f4,
	 0x087e051a,
	 0x3b537050,
	 0x826cce7a}};

unsigned int api_gen_test_work(unsigned int i, unsigned int chip_under_test_num, unsigned int * data)
{
	unsigned int tmp = 0;
	int j;

	for (j = 0; j < 18; j++) {
		data[j] = test_data[i%16][j];
	}

	tmp = data[0];
        data[0] = data[0] ^ (i & 0xfffffff0);//nonce
        data[2] = data[2] - chip_under_test_num;//role time

        data[18] = 0x0;//nonce2
        data[19] = i;//nonce2
        data[20] = 0x1;//cpm2
        data[21] = 0x1;//cpm1
        data[22] = 0x1;//cpm0

	return tmp + 0x18000;
}

void api_set_num(unsigned int ch_num, unsigned int chip_num){
	unsigned int tmp;
	tmp = readl(0x80000510) & 0xff;//reg sck
	tmp = (ch_num << 16) | (chip_num << 24) | tmp;
	writel(tmp, 0x80000510);
}

void api_set_sck(unsigned int spi_speed){
	unsigned int tmp;
	tmp = (readl(0x80000510) & 0xffff0000) | spi_speed;
	writel(tmp, 0x80000510);
}

void api_set_timeout(unsigned int timeout){
	writel(timeout, 0x8000050c);
}

void api_set_flush(){
	writel(0x2, 0x80000508);
}

void api_initial(unsigned int ch_num, unsigned int chip_num, unsigned int spi_speed, unsigned int timeout){
	api_set_num(ch_num, chip_num);
	api_set_sck(spi_speed);
	api_set_flush();
}

unsigned int api_get_tx_cnt(){
	unsigned int tmp = (readl(0x80000508) >> 2) && 0x3ff;
	return tmp;
}

unsigned int api_get_rx_cnt(){
	unsigned int tmp = (readl(0x80000508) >> 20) && 0x1ff;
	return tmp;
}

void api_set_tx_fifo(unsigned int * data){
	int i;
	for(i = 0; i < 23; i++){
		writel(data[i], 0x80000500);
	}
}

void api_get_rx_fifo(unsigned int * data){
	int i;
	for(i = 0; i < 4; i++){
		data[i] = readl(0x80000504);
	}
}

void api_wait_done(unsigned int ch_num, unsigned int chip_num){
	while(api_get_rx_cnt() != (ch_num * chip_num * 4));
}

unsigned int api_verify_nonce(
unsigned int ch_num,
unsigned int chip_num,
unsigned int chip_under_test_num,
unsigned int verify_on,
unsigned int target_nonce
){
	unsigned int i, j, need_verify;
	unsigned int rx_data[4];
	unsigned int pass_cal_num = 0;
	for(i = 0; i < ch_num; i++){
		for(j = 0; j < chip_num; j++){
			api_get_rx_fifo(rx_data);
			need_verify = ((chip_num - chip_under_test_num - 1) == j) ? 1 : 0;
			if(verify_on && (rx_data[2] == target_nonce) && need_verify)
				pass_cal_num++;
		}
	}
	return pass_cal_num;
}

unsigned int api_asic_test(
unsigned int ch_num, 
unsigned int chip_num,
unsigned int cal_core_num
){
	unsigned int i, j, k;
	unsigned int tx_data[23];
	unsigned int target_nonce;
	unsigned int pass_cal_num = 0;
	unsigned int verify_on = 0;
	unsigned int spi_speed = 0x10;
	unsigned int timeout = 0x5000;
	api_initial(ch_num, chip_num, spi_speed, timeout);

	for(i = 0; i < chip_num; i++){
		for(j = 0; j < cal_core_num + 2; j++){
			api_gen_test_work(j, i, tx_data);
			for(k = 0; k < ch_num; k++)
				api_set_tx_fifo(tx_data);

			api_wait_done(ch_num, chip_num);
			target_nonce = api_gen_test_work((j-2)%16, 0, tx_data);
			verify_on = j >= 2 ? 1 : 0;
			pass_cal_num += api_verify_nonce(ch_num, chip_num, i, verify_on, target_nonce);
		}
	}
	return pass_cal_num;
}

void sft_led(unsigned char data)
{
	int i;
	unsigned int tmp = 0;
	tmp = readl(0x80000624) & 0xfffffffc;
	for(i = 0; i < 8; i++){
		writel((tmp & 0xfffffffc) | (data & 1)    , 0x80000624);//clk low

		writel((tmp & 0xfffffffc) | (data & 1) | 2, 0x80000624);//clk high

		writel((tmp & 0xfffffffc) | (data & 1)    , 0x80000624);//clk low
		data = data >> 1;
	}
}

unsigned int api_set_cpm(
unsigned int NR,
unsigned int NF,
unsigned int OD,
unsigned int NB,
unsigned int div
){
/*
	Example:

        NR = 1; NF = 24; OD = 1; NB = 24; div = 2;//300MHz
        NR = 1; NF = 16; OD = 1; NB = 16; div = 1;//400MHz
        NR = 1; NF = 20; OD = 1; NB = 20; div = 1;//500MHz
        NR = 1; NF = 16; OD = 1; NB = 16; div = 2;//200MHz
        data[22] = cpm_cfg(NR, NF, OD, NB, div);
*/
	unsigned int cpm = 0;
	unsigned int div_loc = 0;
	unsigned int NR_sub;
	unsigned int NF_sub;
	unsigned int OD_sub;
	unsigned int NB_sub;
	NR_sub = NR - 1;
	NF_sub = NF - 1;
	OD_sub = OD - 1;
	NB_sub = NB - 1;
	
	if(div == 1  ) div_loc = 0;
	if(div == 2  ) div_loc = 1;
	if(div == 4  ) div_loc = 2;
	if(div == 8  ) div_loc = 3;
	if(div == 16 ) div_loc = 4;
	if(div == 32 ) div_loc = 5;
	if(div == 64 ) div_loc = 6;
	if(div == 128) div_loc = 7;
	
	cpm = 0x7 | (div_loc << 7) | (1<<10) | (NR_sub << 11) | (NF_sub << 15) | (OD_sub << 21) | (NB_sub << 25);
	
	return cpm;
}
