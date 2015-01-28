`include "system_conf.v"
`include "mm_defines.vh"
`include "arbiter2.v"

`define MM_50M

`ifdef MM_50M
`define MM_CLK_IN_MHZ   50
`define MM_CLK_1S_CNT   (27'h5f5e100/2)
`define MM_CLK_PROD     40
`define MM_CLK_MUL      2
`define MM_CLK_DIV      2
`define MM_IIC_GLITCH   4
`define MM_IIC_NEGEDGE_DLY      (62 - `MM_IIC_GLITCH)
`define MM_IIC_POSEDGE_DLY  15
`define MM_IIC_RD_DLY   (13 - `MM_IIC_GLITCH)
`else
`define MM_CLK_IN_MHZ   100
`define MM_CLK_1S_CNT   27'h5f5e100
`define MM_CLK_PROD     40
`define MM_CLK_MUL      4
`define MM_CLK_DIV      4
`define MM_IIC_GLITCH   8
`define MM_IIC_NEGEDGE_DLY      (62 - `MM_IIC_GLITCH)
`define MM_IIC_POSEDGE_DLY  30
`define MM_IIC_RD_DLY   (26 - `MM_IIC_GLITCH)
`endif

`include "../components/lm32_top/lm32_functions.v" // for clogb2_v1
`include "../components/lm32_top/lm32_include.v" // for {IROM,DRAM}_ADDR_WIDTH
`include "../components/lm32_top/lm32_include_all.v"
`include "../components/uart_core/uart_core.v"
`include "../components/sha/sha.v"
`include "../components/sha/dbl_sha.v"
`include "../components/sha/sha_core.v"

`include "../components/twi/twi_define.v"
`include "../components/twi/twi.v"
`include "../components/twi/shift.v"
`include "../components/twi/twi_core.v"
`include "../components/twi/led_ctrl.v"
`include "../components/twi/led_shift.v"

`include "../components/i2c/i2c.v"
`include "../components/i2c/i2c_phy.v"
`include "../components/i2c/brg_shift.v"

`include "../components/api/api_define.v"
`include "../components/api/api.v"
`include "../components/api/api_slave.v"
`include "../components/api/api_ctrl.v"
`include "../components/api/api_phy.v"
`include "../components/api/api_timer.v"

`include "../components/mboot/mboot.v"

module mm (
  ex_clk_i
, ex_clk_o

, gpioPIO_IN
, gpioPIO_OUT
, uart_debugSIN
, uart_debugSOUT
, PWM
, TWI_SCL
, TWI_SDA
, I2C_SCL
, I2C_SDA

, FAN_IN0

, API_LOAD
, API_SCK
, API_MOSI
, API_MISO

, SFTA_SHCP
, SFTA_DS  
, SFTA_STCP
, SFTA_OE_N
 
, SFTB_SHCP
, SFTB_DS  
, SFTB_STCP
, SFTB_OE_N

, SFTC_SHCP
, SFTC_DS

, MBOOT_SCL    
, MBOOT_CS     
, MBOOT_MOSI   
, MBOOT_HOLD_N 
, MBOOT_WP_N   
, MBOOT_MISO   

);
output MBOOT_SCL    ;
output MBOOT_CS     ;
output MBOOT_MOSI   ;
output MBOOT_HOLD_N ;
output MBOOT_WP_N   ;
input  MBOOT_MISO   ;

output [`API_NUM-1:0] API_LOAD ;
output [`API_NUM-1:0] API_SCK  ;
output [`API_NUM-1:0] API_MOSI ;
input  [`API_NUM-1:0] API_MISO ;

output SFTA_SHCP;
output SFTA_DS  ;
output SFTA_STCP;
output SFTA_OE_N;

output SFTB_SHCP;
output SFTB_DS  ;
output SFTB_STCP;
output SFTB_OE_N;

output SFTC_SHCP;
output SFTC_DS  ;

output PWM ;
input	ex_clk_i;
output  [1:0] ex_clk_o ;
wire clk_i , reset_n, clk25m_on;

clkgen clk (.clkin(ex_clk_i), .clk25m_on(clk25m_on), .clkout(clk_i), .clk25m(ex_clk_o), .locked(reset_n));

wire                rbt_enable;
wire                ram_sel   ;//0: iram, 1: dram
wire                ram_wr    ;
wire [15:0]         ram_addr  ;
wire [31:0]         ram_dat_wr;
wire [31:0]         ram_dat_rd;

wire WATCH_DOG ;
wire [31:0] irom_q_rd, irom_q_wr;
wire [31:0] dram_q_rd, dram_q_wr /* unused */;
wire irom_clk_rd, irom_clk_wr;
wire irom_rst_rd, irom_rst_wr;
wire [31:0] irom_d_rd /* unused */, irom_d_wr;
wire [32-2-1:0] irom_addr_rd, irom_addr_wr;
wire irom_en_rd, irom_en_wr;
wire irom_write_rd, irom_write_wr;
wire dram_clk_rd, dram_clk_wr;
wire dram_rst_rd, dram_rst_wr;
wire [31:0] dram_d_rd /* unused */, dram_d_wr;
wire [32-2-1:0] dram_addr_rd, dram_addr_wr;
wire dram_en_rd, dram_en_wr;
wire dram_write_rd, dram_write_wr;

genvar i;
wire [31:0] zwire = 32'hZZZZZZZZ;
wire [31:0] zerowire = 32'h00000000;
wire [31:0] SHAREDBUS_ADR_I;
wire [31:0] SHAREDBUS_DAT_I;
wire [31:0] SHAREDBUS_DAT_O;
wire [3:0] SHAREDBUS_SEL_I;
wire   SHAREDBUS_WE_I;
wire   SHAREDBUS_ACK_O;
wire   SHAREDBUS_ERR_O;
wire   SHAREDBUS_RTY_O;
wire [2:0] SHAREDBUS_CTI_I;
wire [1:0] SHAREDBUS_BTE_I;
wire   SHAREDBUS_LOCK_I;
wire   SHAREDBUS_CYC_I;
wire   SHAREDBUS_STB_I;
wire SHAREDBUS_en;

wire [31:0] superkdf9I_ADR_O;
wire [31:0] superkdf9I_DAT_O;
wire [31:0] superkdf9I_DAT_I;
wire [3:0] superkdf9I_SEL_O;
wire   superkdf9I_WE_O;
wire   superkdf9I_ACK_I;
wire   superkdf9I_ERR_I;
wire   superkdf9I_RTY_I;
wire [2:0] superkdf9I_CTI_O;
wire [1:0] superkdf9I_BTE_O;
wire   superkdf9I_LOCK_O;
wire   superkdf9I_CYC_O;
wire   superkdf9I_STB_O;
wire [31:0] superkdf9D_ADR_O;
wire [31:0] superkdf9D_DAT_O;
wire [31:0] superkdf9D_DAT_I;
wire [3:0] superkdf9D_SEL_O;
wire   superkdf9D_WE_O;
wire   superkdf9D_ACK_I;
wire   superkdf9D_ERR_I;
wire   superkdf9D_RTY_I;
wire [2:0] superkdf9D_CTI_O;
wire [1:0] superkdf9D_BTE_O;
wire   superkdf9D_LOCK_O;
wire   superkdf9D_CYC_O;
wire   superkdf9D_STB_O;
wire superkdf9DEBUG_en;
wire [31:0] superkdf9interrupt_n;

wire [31:0] uartUART_DAT_O;
wire   uartUART_ACK_O;
wire   uartUART_ERR_O;
wire   uartUART_RTY_O;
wire uartUART_en;

wire i2cI2C_ACK_O, i2cI2C_ERR_O, i2cI2C_RTY_O;
wire [31:0] i2cI2C_DAT_O; 

input [9:0] gpioPIO_IN;
output [7:0] gpioPIO_OUT;

wire [7:0] uart_debugUART_DAT_O;
wire   uart_debugUART_ACK_O;
wire   uart_debugUART_ERR_O;
wire   uart_debugUART_RTY_O;
wire uart_debugUART_en;
wire uart_debugINTR;
input  uart_debugSIN;
output  uart_debugSOUT;

//sha core
wire [31:0] shaSHA_DAT_O;
wire   shaSHA_ACK_O;
wire   shaSHA_ERR_O;
wire   shaSHA_RTY_O;
wire   shaSHA_en;

//alink core
wire [31:0] alinkALINK_DAT_O;
wire        alinkALINK_ACK_O;
wire        alinkALINK_ERR_O;
wire        alinkALINK_RTY_O;
wire        alinkALINK_en;

//twi core
inout       TWI_SCL ;
inout       TWI_SDA ;
input       I2C_SCL ;
inout       I2C_SDA ;

wire [31:0] twiTWI_DAT_O;
wire        twiTWI_ACK_O;
wire        twiTWI_ERR_O;
wire        twiTWI_RTY_O;
wire        twiTWI_en;
wire        TWI_SCL_O ;
wire        TWI_SDA_OEN ;

wire [31:0] mbootMBOOT_DAT_O;
wire        mbootMBOOT_ACK_O;
wire        mbootMBOOT_ERR_O;
wire        mbootMBOOT_RTY_O;
wire        mbootMBOOT_en;

wire brg_en;
wire brg_cs;
wire brg_sck;                                                                                            
wire brg_mosi;

input       FAN_IN0 ;
// Enable the FT232 and HUB
wire TIME0_INT ;
wire TIME1_INT ;
reg [2:0] counter;
wire sys_reset = !counter[2] || WATCH_DOG ;
always @(posedge clk_i or negedge reset_n) begin
	if (reset_n == 1'b0)
		counter <= 3'b000;
	else if (counter[2] == 1'b0)
		counter <= counter + 3'b1;
end

wire one_zero = 1'b0;
wire[1:0] two_zero = 2'b00;
wire[2:0] three_zero = 3'b000;
wire[3:0] four_zero = 4'b0000;
wire[31:0] thirtytwo_zero = 32'b0000_0000_0000_0000_0000_0000_0000_0000;

arbiter2
#(
.MAX_DAT_WIDTH ( 32 )
,.WBS_DAT_WIDTH ( 32 )
,.WBM0_DAT_WIDTH ( 32 )
,.WBM1_DAT_WIDTH ( 32 )
)
arbiter (
.WBM0_ADR_O(superkdf9I_ADR_O),
.WBM0_DAT_O(superkdf9I_DAT_O[31:0]),
.WBM0_DAT_I(superkdf9I_DAT_I),
.WBM0_SEL_O(superkdf9I_SEL_O[3:0]),
.WBM0_WE_O(superkdf9I_WE_O),
.WBM0_ACK_I(superkdf9I_ACK_I),
.WBM0_ERR_I(superkdf9I_ERR_I),
.WBM0_RTY_I(superkdf9I_RTY_I),
.WBM0_CTI_O(superkdf9I_CTI_O),
.WBM0_BTE_O(superkdf9I_BTE_O),
.WBM0_LOCK_O(superkdf9I_LOCK_O),
.WBM0_CYC_O(superkdf9I_CYC_O),
.WBM0_STB_O(superkdf9I_STB_O),
.WBM1_ADR_O(superkdf9D_ADR_O),
.WBM1_DAT_O(superkdf9D_DAT_O[31:0]),
.WBM1_DAT_I(superkdf9D_DAT_I),
.WBM1_SEL_O(superkdf9D_SEL_O[3:0]),
.WBM1_WE_O(superkdf9D_WE_O),
.WBM1_ACK_I(superkdf9D_ACK_I),
.WBM1_ERR_I(superkdf9D_ERR_I),
.WBM1_RTY_I(superkdf9D_RTY_I),
.WBM1_CTI_O(superkdf9D_CTI_O),
.WBM1_BTE_O(superkdf9D_BTE_O),
.WBM1_LOCK_O(superkdf9D_LOCK_O),
.WBM1_CYC_O(superkdf9D_CYC_O),
.WBM1_STB_O(superkdf9D_STB_O),
.WBS_ADR_I(SHAREDBUS_ADR_I[31:0]),
.WBS_DAT_I(SHAREDBUS_DAT_I[31:0]),
.WBS_DAT_O(SHAREDBUS_DAT_O[31:0]),
.WBS_SEL_I(SHAREDBUS_SEL_I[3:0]),
.WBS_WE_I(SHAREDBUS_WE_I),
.WBS_ACK_O(SHAREDBUS_ACK_O),
.WBS_ERR_O(SHAREDBUS_ERR_O),
.WBS_RTY_O(SHAREDBUS_RTY_O),
.WBS_CTI_I(SHAREDBUS_CTI_I),
.WBS_BTE_I(SHAREDBUS_BTE_I),
.WBS_LOCK_I(SHAREDBUS_LOCK_I),
.WBS_CYC_I(SHAREDBUS_CYC_I),
.WBS_STB_I(SHAREDBUS_STB_I),
.clk (clk_i),
.reset (sys_reset | rbt_enable));
assign SHAREDBUS_DAT_O =
uartUART_en ? {4{uartUART_DAT_O[7:0]}} :
uart_debugUART_en ? {4{uart_debugUART_DAT_O[7:0]}} :
shaSHA_en ? shaSHA_DAT_O :
alinkALINK_en ? alinkALINK_DAT_O :
twiTWI_en ? twiTWI_DAT_O :
mbootMBOOT_en ? mbootMBOOT_DAT_O :
i2cI2C_en ? i2cI2C_DAT_O :
0;
assign SHAREDBUS_ERR_O = SHAREDBUS_CYC_I & !(
(!uartUART_ERR_O & uartUART_en) |
(!uart_debugUART_ERR_O & uart_debugUART_en) |
(!shaSHA_ERR_O & shaSHA_en ) |
(!alinkALINK_ERR_O & alinkALINK_en ) |
(!twiTWI_ERR_O & twiTWI_en ) |
(!mbootMBOOT_ERR_O & mbootMBOOT_en ) |
(!i2cI2C_ERR_O & i2cI2C_en ) |
0);
assign SHAREDBUS_ACK_O =
uartUART_en ? uartUART_ACK_O :
uart_debugUART_en ? uart_debugUART_ACK_O :
shaSHA_en ? shaSHA_ACK_O :
alinkALINK_en ? alinkALINK_ACK_O :
twiTWI_en ? twiTWI_ACK_O :
mbootMBOOT_en ? mbootMBOOT_ACK_O :
i2cI2C_en ? i2cI2C_ACK_O :
1'b0;
assign SHAREDBUS_RTY_O =
uartUART_en ? uartUART_RTY_O :
uart_debugUART_en ? uart_debugUART_RTY_O :
shaSHA_en ? shaSHA_RTY_O :
alinkALINK_en ? alinkALINK_RTY_O :
twiTWI_en ? twiTWI_RTY_O :
mbootMBOOT_en ? mbootMBOOT_RTY_O :
i2cI2C_en ? i2cI2C_RTY_O :
1'b0;

wire [31:0] superkdf9DEBUG_DAT_I;
assign superkdf9DEBUG_DAT_I = 0;
wire [3:0] superkdf9DEBUG_SEL_I;
assign superkdf9DEBUG_SEL_I = 0;
assign superkdf9DEBUG_en = 0;
lm32_top
 superkdf9(
.I_ADR_O(superkdf9I_ADR_O),
.I_DAT_O(superkdf9I_DAT_O),
.I_DAT_I(superkdf9I_DAT_I),
.I_SEL_O(superkdf9I_SEL_O),
.I_WE_O(superkdf9I_WE_O),
.I_ACK_I(superkdf9I_ACK_I),
.I_ERR_I(superkdf9I_ERR_I),
.I_RTY_I(superkdf9I_RTY_I),
.I_CTI_O(superkdf9I_CTI_O),
.I_BTE_O(superkdf9I_BTE_O),
.I_LOCK_O(superkdf9I_LOCK_O),
.I_CYC_O(superkdf9I_CYC_O),
.I_STB_O(superkdf9I_STB_O),
.D_ADR_O(superkdf9D_ADR_O),
.D_DAT_O(superkdf9D_DAT_O),
.D_DAT_I(superkdf9D_DAT_I),
.D_SEL_O(superkdf9D_SEL_O),
.D_WE_O(superkdf9D_WE_O),
.D_ACK_I(superkdf9D_ACK_I),
.D_ERR_I(superkdf9D_ERR_I),
.D_RTY_I(superkdf9D_RTY_I),
.D_CTI_O(superkdf9D_CTI_O),
.D_BTE_O(superkdf9D_BTE_O),
.D_LOCK_O(superkdf9D_LOCK_O),
.D_CYC_O(superkdf9D_CYC_O),
.D_STB_O(superkdf9D_STB_O),
.DEBUG_ADR_I(SHAREDBUS_ADR_I[31:0]),
.DEBUG_DAT_I(superkdf9DEBUG_DAT_I[31:0]),
.DEBUG_DAT_O(),
.DEBUG_SEL_I(superkdf9DEBUG_SEL_I[3:0]),
.DEBUG_WE_I(SHAREDBUS_WE_I),
.DEBUG_ACK_O(),
.DEBUG_ERR_O(),
.DEBUG_RTY_O(),
.DEBUG_CTI_I(SHAREDBUS_CTI_I),
.DEBUG_BTE_I(SHAREDBUS_BTE_I),
.DEBUG_LOCK_I(SHAREDBUS_LOCK_I),
.DEBUG_CYC_I(SHAREDBUS_CYC_I & superkdf9DEBUG_en),
.DEBUG_STB_I(SHAREDBUS_STB_I & superkdf9DEBUG_en),
.interrupt_n(superkdf9interrupt_n),
.clk_i (clk_i), .rst_i (sys_reset | rbt_enable),
.user_result(), .user_complete(),
// the exposed IROM
.irom_clk_rd(irom_clk_rd),
.irom_clk_wr(irom_clk_wr),
.irom_rst_rd(irom_rst_rd),
.irom_rst_wr(irom_rst_wr),
.irom_d_rd(irom_d_rd) /* unused */,
.irom_d_wr(irom_d_wr),
.irom_q_rd(irom_q_rd),
.irom_q_wr(irom_q_wr),
.irom_addr_rd(irom_addr_rd[`IROM_ADDR_WIDTH-1:0]),
.irom_addr_wr(irom_addr_wr[`IROM_ADDR_WIDTH-1:0]),
.irom_en_rd(irom_en_rd),
.irom_en_wr(irom_en_wr),
.irom_write_rd(irom_write_rd),
.irom_write_wr(irom_write_wr),
// the exposed DRAM
.dram_clk_rd(dram_clk_rd),
.dram_clk_wr(dram_clk_wr),
.dram_rst_rd(dram_rst_rd),
.dram_rst_wr(dram_rst_wr),
.dram_d_rd(dram_d_rd) /* unused */,
.dram_d_wr(dram_d_wr),
.dram_q_rd(dram_q_rd),
.dram_q_wr(dram_q_wr) /* unused */,
.dram_addr_rd(dram_addr_rd[`DRAM_ADDR_WIDTH-1:0]),
.dram_addr_wr(dram_addr_wr[`DRAM_ADDR_WIDTH-1:0]),
.dram_en_rd(dram_en_rd),
.dram_en_wr(dram_en_wr),
.dram_write_rd(dram_write_rd),
.dram_write_wr(dram_write_wr)
);


assign ram_dat_rd = ram_sel ? dram_q_rd : irom_q_rd;

bram #(
	.size(1+`CFG_IROM_LIMIT - `CFG_IROM_BASE_ADDRESS),
	.name("irom")
) irom (
	.ClockA  (irom_clk_rd  ),
	.ClockB  (irom_clk_wr  ),
	.ResetA  (rbt_enable ? 1'b0 : irom_rst_rd),
	.ResetB  (rbt_enable ? 1'b0 : irom_rst_wr),
	.AddressA(rbt_enable ? {16'b0, ram_addr} : irom_addr_rd ),
	.AddressB(irom_addr_wr ),
	.DataInA (rbt_enable ? ram_dat_wr : irom_d_rd    ), /* unused */
	.DataInB (irom_d_wr    ),
	.DataOutA(irom_q_rd    ),
	.DataOutB(irom_q_wr    ), /* unused */
	.ClockEnA(rbt_enable ? 1'b1 : irom_en_rd   ),
	.ClockEnB(irom_en_wr   ),
	.WriteA  (rbt_enable ? (~ram_sel&ram_wr) : irom_write_rd),
	.WriteB  (rbt_enable ? 1'b0 : irom_write_wr)
);

bram #(
	.size(1+`CFG_DRAM_LIMIT - `CFG_DRAM_BASE_ADDRESS),
	.name("irom")
) dram (
	.ClockA  (dram_clk_rd  ),
	.ClockB  (dram_clk_wr  ),
	.ResetA  (rbt_enable ? 1'b0 : dram_rst_rd  ),
	.ResetB  (rbt_enable ? 1'b0 : dram_rst_wr  ),
	.AddressA(rbt_enable ? {16'b0, ram_addr} : dram_addr_rd ),
	.AddressB(dram_addr_wr ),
	.DataInA (rbt_enable ? ram_dat_wr : dram_d_rd    ), /* unused */
	.DataInB (dram_d_wr    ),
	.DataOutA(dram_q_rd    ),
	.DataOutB(dram_q_wr    ),
	.ClockEnA(rbt_enable ? 1'b1 : dram_en_rd   ),
	.ClockEnB(dram_en_wr   ),
	.WriteA  (rbt_enable ? (ram_sel&ram_wr) : dram_write_rd),
	.WriteB  (rbt_enable ? 1'b0 : dram_write_wr)
);


assign uartUART_en = (SHAREDBUS_ADR_I[31:4] == 28'b1000000000000000000000010000);

wire led_iic_wr, led_iic_rd, led_get_nonce_l, led_get_nonce_h;

reg uartUART_ACK_O_r;
assign uartUART_ACK_O = uartUART_ACK_O_r;
assign uartUART_DAT_O = 0;
assign uartUART_ERR_O = 0;
assign uartUART_RTY_O = 0;

always @ ( posedge clk_i or posedge sys_reset ) begin
	if( sys_reset )
		uartUART_ACK_O_r <= 1'b0 ;
	else if( (SHAREDBUS_STB_I & uartUART_en) && (~uartUART_ACK_O_r) )
		uartUART_ACK_O_r <= 1'b1 ;
	else
		uartUART_ACK_O_r <= 1'b0 ;
end

assign i2cI2C_en = (SHAREDBUS_ADR_I[31:5] == 27'b100000000000000000000111000);

i2c i2c_slv(
/*input            */ .CLK_I     (clk_i),
/*input            */ .RST_I     (sys_reset),

/*input            */ .I2C_CYC_I (SHAREDBUS_CYC_I & i2cI2C_en),//NC
/*input            */ .I2C_STB_I (SHAREDBUS_STB_I & i2cI2C_en),
/*input            */ .I2C_WE_I  (SHAREDBUS_WE_I),
/*input            */ .I2C_LOCK_I(SHAREDBUS_LOCK_I),//NC
/*input  [2:0]     */ .I2C_CTI_I (SHAREDBUS_CTI_I),//NC
/*input  [1:0]     */ .I2C_BTE_I (SHAREDBUS_BTE_I),//NC
/*input  [5:0]     */ .I2C_ADR_I (SHAREDBUS_ADR_I[5:0]),
/*input  [31:0]    */ .I2C_DAT_I (SHAREDBUS_DAT_I[31:0]),
/*input  [3:0]     */ .I2C_SEL_I (SHAREDBUS_SEL_I),
/*output reg       */ .I2C_ACK_O (i2cI2C_ACK_O),
/*output           */ .I2C_ERR_O (i2cI2C_ERR_O),//const 0
/*output           */ .I2C_RTY_O (i2cI2C_RTY_O),//const 0
/*output reg [31:0]*/ .I2C_DAT_O (i2cI2C_DAT_O),

/*input            */ .scl_pin   (I2C_SCL),
/*inout            */ .sda_pin   (I2C_SDA),

/*output           */ .int_i2c   (int_i2c                    ),

/*output           */ .rbt_enable(rbt_enable                 ),
/*output           */ .ram_sel   (ram_sel                    ),//0: iram, 1: dram
/*output           */ .ram_wr    (ram_wr                     ),
/*output [15:0]    */ .ram_addr  (ram_addr                   ),
/*output [31:0]    */ .ram_dat_wr(ram_dat_wr                 ),
/*input  [31:0]    */ .ram_dat_rd(ram_dat_rd                 ),

/*output           */ .led_iic_wr(led_iic_wr),
/*output           */ .led_iic_rd(led_iic_rd),

/*output           */ .brg_en  (brg_en  ),
/*output           */ .brg_cs  (brg_cs  ),
/*output           */ .brg_sck (brg_sck ),                                                                                            
/*output           */ .brg_mosi(brg_mosi) 
 
);

wire [7:0] uart_debugUART_DAT_I;
assign uart_debugUART_DAT_I = ((
	SHAREDBUS_ADR_I[1:0] == 2'b00) ? SHAREDBUS_DAT_I[31:24] : ((
	SHAREDBUS_ADR_I[1:0] == 2'b01) ? SHAREDBUS_DAT_I[23:16] : ((
	SHAREDBUS_ADR_I[1:0] == 2'b10) ? SHAREDBUS_DAT_I[15:8] : SHAREDBUS_DAT_I[7:0])));
wire uart_debugUART_SEL_I;
assign uart_debugUART_SEL_I = ((
	SHAREDBUS_ADR_I[1:0] == 2'b00) ? SHAREDBUS_SEL_I[3] : ((
	SHAREDBUS_ADR_I[1:0] == 2'b01) ? SHAREDBUS_SEL_I[2] : ((
	SHAREDBUS_ADR_I[1:0] == 2'b10) ? SHAREDBUS_SEL_I[1] : SHAREDBUS_SEL_I[0])));
assign uart_debugUART_en = (SHAREDBUS_ADR_I[31:4] == 28'b1000000000000000000000110000);
uart_core
#(
.UART_WB_DAT_WIDTH(8),
.UART_WB_ADR_WIDTH(4),
.CLK_IN_MHZ(`MM_CLK_IN_MHZ),
.BAUD_RATE(115200),
.STDOUT_SIM(0),
.STDOUT_SIMFAST(0),
.LCR_DATA_BITS(8),
.LCR_STOP_BITS(1),
.LCR_PARITY_ENABLE(0),
.LCR_PARITY_ODD(0),
.LCR_PARITY_STICK(0),
.LCR_SET_BREAK(0),
.FIFO(1))
 uart_debug(
.UART_ADR_I(SHAREDBUS_ADR_I[3:0]),
.UART_DAT_I(uart_debugUART_DAT_I[7:0]),
.UART_DAT_O(uart_debugUART_DAT_O[7:0]),
.UART_SEL_I(uart_debugUART_SEL_I),
.UART_WE_I(SHAREDBUS_WE_I),
.UART_ACK_O(uart_debugUART_ACK_O),
.UART_ERR_O(uart_debugUART_ERR_O),
.UART_RTY_O(uart_debugUART_RTY_O),
.UART_CTI_I(SHAREDBUS_CTI_I),
.UART_BTE_I(SHAREDBUS_BTE_I),
.UART_LOCK_I(SHAREDBUS_LOCK_I),
.UART_CYC_I(SHAREDBUS_CYC_I & uart_debugUART_en),
.UART_STB_I(SHAREDBUS_STB_I & uart_debugUART_en),
.SIN(uart_debugSIN),
.SOUT(uart_debugSOUT),
.INTR(uart_debugINTR),
.CLK(clk_i), 
.RESET(sys_reset),
.RXRDY_N(), .TXRDY_N());

assign shaSHA_en = (SHAREDBUS_ADR_I[31:5] == 28'b100000000000000000000100000);
sha sha256(
// system clock and reset
/*input        */ .CLK_I     (clk_i),
/*input        */ .RST_I     (sys_reset),

// wishbone interface signals
/*input        */ .SHA_CYC_I (SHAREDBUS_CYC_I & shaSHA_en ) ,//NC
/*input        */ .SHA_STB_I (SHAREDBUS_STB_I & shaSHA_en ) ,
/*input        */ .SHA_WE_I  (SHAREDBUS_WE_I              ) ,
/*input        */ .SHA_LOCK_I(SHAREDBUS_LOCK_I            ) ,//NC
/*input [2:0]  */ .SHA_CTI_I (SHAREDBUS_CTI_I             ) ,//NC
/*input [1:0]  */ .SHA_BTE_I (SHAREDBUS_BTE_I             ) ,//NC
/*input [4:0]  */ .SHA_ADR_I (SHAREDBUS_ADR_I[4:0]        ) ,
/*input [31:0] */ .SHA_DAT_I (SHAREDBUS_DAT_I[31:0]       ) ,
/*input [3:0]  */ .SHA_SEL_I (SHAREDBUS_SEL_I             ) ,
/*output reg   */ .SHA_ACK_O (shaSHA_ACK_O                ) ,
/*output       */ .SHA_ERR_O (shaSHA_ERR_O                ) ,//const 0
/*output       */ .SHA_RTY_O (shaSHA_RTY_O                ) ,//const 0
/*output [31:0]*/ .SHA_DAT_O (shaSHA_DAT_O                )
);

assign alinkALINK_en = (SHAREDBUS_ADR_I[31:6] == 26'b10000000000000000000010100);

wire api_sck_w;
wire api_mosi_w;
wire api_idle;
assign API_SCK = {`API_NUM{api_sck_w}};
assign API_MOSI = {`API_NUM{api_mosi_w}};

api api(
// system clock and reset
/*input         */ .CLK_I       (clk_i) ,
/*input         */ .RST_I       (sys_reset) ,

// wishbone interface signals
/*input         */ .API_CYC_I (SHAREDBUS_CYC_I & alinkALINK_en ) ,//NC
/*input         */ .API_STB_I (SHAREDBUS_STB_I & alinkALINK_en ) ,
/*input         */ .API_WE_I  (SHAREDBUS_WE_I                  ) ,
/*input         */ .API_LOCK_I(SHAREDBUS_LOCK_I                ) ,//NC
/*input  [2:0]  */ .API_CTI_I (SHAREDBUS_CTI_I                 ) ,//NC
/*input  [1:0]  */ .API_BTE_I (SHAREDBUS_BTE_I                 ) ,//NC
/*input  [5:0]  */ .API_ADR_I (SHAREDBUS_ADR_I[5:0]            ) ,
/*input  [31:0] */ .API_DAT_I (SHAREDBUS_DAT_I[31:0]           ) ,
/*input  [3:0]  */ .API_SEL_I (SHAREDBUS_SEL_I                 ) ,
/*output        */ .API_ACK_O (alinkALINK_ACK_O                ) ,
/*output        */ .API_ERR_O (alinkALINK_ERR_O                ) ,//const 0
/*output        */ .API_RTY_O (alinkALINK_RTY_O                ) ,//const 0
/*output [31:0] */ .API_DAT_O (alinkALINK_DAT_O                ) ,

/*output [`API_NUM-1:0]*/ .load (API_LOAD   ),
/*output               */ .sck  (api_sck_w  ),
/*output               */ .mosi (api_mosi_w ),
/*input  [`API_NUM-1:0]*/ .miso (API_MISO   ),

/*output               */ .led_get_nonce_l(led_get_nonce_l),
/*output               */ .led_get_nonce_h(led_get_nonce_h),
/*output               */ .api_idle       (api_idle       )
);

wire [7:0] gpioPIO_OUT_fake;
assign twiTWI_en = (SHAREDBUS_ADR_I[31:6] == 26'b10000000000000000000011000);
assign TWI_SCL = TWI_SCL_O == 1'b0 ? 1'b0 : 1'bz ;//p85
assign TWI_SDA = TWI_SDA_OEN == 1'b0 ? 1'b0 : 1'bz ;//p8
twi u_twi(
// system clock and reset
/*input         */ .CLK_I       (clk_i                       ) ,
/*input         */ .RST_I       (sys_reset                   ) ,

// wishbone interface signals
/*input         */ .TWI_CYC_I   (SHAREDBUS_CYC_I & twiTWI_en ) ,//NC
/*input         */ .TWI_STB_I   (SHAREDBUS_STB_I & twiTWI_en ) ,
/*input         */ .TWI_WE_I    (SHAREDBUS_WE_I              ) ,
/*input         */ .TWI_LOCK_I  (SHAREDBUS_LOCK_I            ) ,//NC
/*input  [2:0]  */ .TWI_CTI_I   (SHAREDBUS_CTI_I             ) ,//NC
/*input  [1:0]  */ .TWI_BTE_I   (SHAREDBUS_BTE_I             ) ,//NC
/*input  [5:0]  */ .TWI_ADR_I   (SHAREDBUS_ADR_I[5:0]        ) ,
/*input  [31:0] */ .TWI_DAT_I   (SHAREDBUS_DAT_I[31:0]       ) ,
/*input  [3:0]  */ .TWI_SEL_I   (SHAREDBUS_SEL_I             ) ,
/*output reg    */ .TWI_ACK_O   (twiTWI_ACK_O                ) ,
/*output        */ .TWI_ERR_O   (twiTWI_ERR_O                ) ,//const 0
/*output        */ .TWI_RTY_O   (twiTWI_RTY_O                ) ,//const 0
/*output [31:0] */ .TWI_DAT_O   (twiTWI_DAT_O                ) ,

/*output        */ .TWI_SCL_O   (TWI_SCL_O                   ) ,
/*input         */ .TWI_SDA_I   (TWI_SDA                     ) ,
/*output        */ .TWI_SDA_OEN (TWI_SDA_OEN                 ) ,
/*output        */ .PWM         (PWM                         ) ,
/*output        */ .WATCH_DOG   (WATCH_DOG                   ) ,

/*output        */ .SFT_SHCP    (SFTA_SHCP                   ) ,
/*output        */ .SFT_DS      (SFTA_DS                     ) ,
/*output        */ .SFT_STCP    (SFTA_STCP                   ) ,
/*output        */ .SFT_MR_N    (                            ) ,
/*output        */ .SFT_OE_N    (SFTA_OE_N                   ) ,

/*output        */ .SFTB_SHCP   (SFTB_SHCP                   ) ,
/*output        */ .SFTB_DS     (SFTB_DS                     ) ,
/*output        */ .SFTB_STCP   (SFTB_STCP                   ) ,
/*output        */ .SFTB_MR_N   (                            ) ,
/*output        */ .SFTB_OE_N   (SFTB_OE_N                   ) ,

/*output        */ .SFTC_SHCP   (SFTC_SHCP                   ) ,
/*output        */ .SFTC_DS     (SFTC_DS                     ) ,

/*input         */ .FAN_IN0     (FAN_IN0                     ) ,
/*output        */ .TIME0_INT   (TIME0_INT                   ) ,
/*output        */ .TIME1_INT   (TIME1_INT                   ) ,
/*output [15:0] */ .GPIO_OUT    ({gpioPIO_OUT_fake[7:0], gpioPIO_OUT[7:0]}) ,
/*input  [15:0] */ .GPIO_IN     ({6'b0, gpioPIO_IN[9:0]}     ) ,
/*output        */ .clk25m_on   (clk25m_on                   ) ,

/*input  [3:0]  */ .led_bling   ({led_get_nonce_h, led_get_nonce_l, led_iic_rd, led_iic_wr}),
/*input         */ .api_idle    (api_idle                    )
) ;

assign mbootMBOOT_en = (SHAREDBUS_ADR_I[31:6] == 26'b10000000000000000000100000);
wire [18:0] mbootMBOOT_DAT_O_tmp;
assign mbootMBOOT_DAT_O = {13'b0, mbootMBOOT_DAT_O_tmp};
mboot mboot(
/*input            */ .CLK_I       (clk_i                          ),
/*input            */ .RST_I       (sys_reset                      ),

/*input            */ .MBOOT_CYC_I (SHAREDBUS_CYC_I & mbootMBOOT_en),//NC
/*input            */ .MBOOT_STB_I (SHAREDBUS_STB_I & mbootMBOOT_en),
/*input            */ .MBOOT_WE_I  (SHAREDBUS_WE_I                 ),
/*input            */ .MBOOT_LOCK_I(SHAREDBUS_LOCK_I               ),//NC
/*input  [2:0]     */ .MBOOT_CTI_I (SHAREDBUS_CTI_I                ),//NC
/*input  [1:0]     */ .MBOOT_BTE_I (SHAREDBUS_BTE_I                ),//NC
/*input  [5:0]     */ .MBOOT_ADR_I (SHAREDBUS_ADR_I[5:0]           ),
/*input  [31:0]    */ .MBOOT_DAT_I (SHAREDBUS_DAT_I[31:0]          ),
/*input  [3:0]     */ .MBOOT_SEL_I (SHAREDBUS_SEL_I                ),
/*output reg       */ .MBOOT_ACK_O (mbootMBOOT_ACK_O               ),
/*output           */ .MBOOT_ERR_O (mbootMBOOT_ERR_O               ),//const 0
/*output           */ .MBOOT_RTY_O (mbootMBOOT_RTY_O               ),//const 0
/*output reg [18:0]*/ .MBOOT_DAT_O (mbootMBOOT_DAT_O_tmp           ),

/*output reg       */ .MBOOT_SCL   (MBOOT_SCL_w                    ),
/*output reg       */ .MBOOT_CS    (MBOOT_CS_w                     ),
/*output reg       */ .MBOOT_MOSI  (MBOOT_MOSI_w                   ),
/*output reg       */ .MBOOT_HOLD_N(MBOOT_HOLD_N_w                 ),
/*output reg       */ .MBOOT_WP_N  (MBOOT_WP_N_w                   ),
/*input            */ .MBOOT_MISO  (MBOOT_MISO                     )
);

assign MBOOT_SCL    = brg_en ? brg_sck  : MBOOT_SCL_w    ;
assign MBOOT_CS     = brg_en ? brg_cs   : MBOOT_CS_w     ;
assign MBOOT_MOSI   = brg_en ? brg_mosi : MBOOT_MOSI_w   ;
assign MBOOT_HOLD_N = brg_en ? 1'b1     : MBOOT_HOLD_N_w ;
assign MBOOT_WP_N   = brg_en ? 1'b1     : MBOOT_WP_N_w   ;

assign superkdf9interrupt_n[3] = 1'b1;
assign superkdf9interrupt_n[1] = 1'b1;
assign superkdf9interrupt_n[0] = 1'b1;
assign superkdf9interrupt_n[4] = brg_en || !uart_debugINTR ;
assign superkdf9interrupt_n[2] = brg_en || !int_i2c;
assign superkdf9interrupt_n[5] = brg_en || !TIME0_INT;
assign superkdf9interrupt_n[6] = brg_en || !TIME1_INT;
assign superkdf9interrupt_n[7] = 1'b1;
assign superkdf9interrupt_n[8] = 1'b1;
assign superkdf9interrupt_n[9] = 1'b1;
assign superkdf9interrupt_n[10] = 1'b1;
assign superkdf9interrupt_n[11] = 1'b1;
assign superkdf9interrupt_n[12] = 1'b1;
assign superkdf9interrupt_n[13] = 1'b1;
assign superkdf9interrupt_n[14] = 1'b1;
assign superkdf9interrupt_n[15] = 1'b1;
assign superkdf9interrupt_n[16] = 1'b1;
assign superkdf9interrupt_n[17] = 1'b1;
assign superkdf9interrupt_n[18] = 1'b1;
assign superkdf9interrupt_n[19] = 1'b1;
assign superkdf9interrupt_n[20] = 1'b1;
assign superkdf9interrupt_n[21] = 1'b1;
assign superkdf9interrupt_n[22] = 1'b1;
assign superkdf9interrupt_n[23] = 1'b1;
assign superkdf9interrupt_n[24] = 1'b1;
assign superkdf9interrupt_n[25] = 1'b1;
assign superkdf9interrupt_n[26] = 1'b1;
assign superkdf9interrupt_n[27] = 1'b1;
assign superkdf9interrupt_n[28] = 1'b1;
assign superkdf9interrupt_n[29] = 1'b1;
assign superkdf9interrupt_n[30] = 1'b1;
assign superkdf9interrupt_n[31] = 1'b1;
endmodule
