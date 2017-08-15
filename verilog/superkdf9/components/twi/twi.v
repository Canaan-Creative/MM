`include "twi_define.v"
module twi(
    // system clock and reset
    input          CLK_I       ,
    input          RST_I       ,

    // wishbone interface signals
    input          TWI_CYC_I   ,//NC
    input          TWI_STB_I   ,
    input          TWI_WE_I    ,
    input          TWI_LOCK_I  ,//NC
    input  [2:0]   TWI_CTI_I   ,//NC
    input  [1:0]   TWI_BTE_I   ,//NC
    input  [5:0]   TWI_ADR_I   ,
    input  [31:0]  TWI_DAT_I   ,
    input  [3:0]   TWI_SEL_I   ,
    output reg     TWI_ACK_O   ,
    output         TWI_ERR_O   ,//const 0
    output         TWI_RTY_O   ,//const 0
    output [31:0]  TWI_DAT_O   ,

    output         TWI_SCL_O   ,
    input          TWI_SDA_I   ,
    output         TWI_SDA_OEN ,
    output         PWM         ,
    output         WATCH_DOG   ,

    output [3:0]   SFT_SHCP    ,
    output [3:0]   SFT_DS      ,
    output [3:0]   SFT_STCP    ,
    output [3:0]   SFT_MR_N    ,
    output [3:0]   SFT_OE_N    ,

    input          FAN_IN0     ,
    input          FAN_IN1     ,
    output         TIME0_INT   ,
    output         TIME1_INT   ,

    output [15:0]  GPIO_OUT    ,
    input  [15:0]  GPIO_IN     ,
    output [7:0]   clk25m_on   ,

    input  [3:0]   led_bling   ,
    input          api_idle    
);

assign TWI_ERR_O = 1'b0 ;
assign TWI_RTY_O = 1'b0 ;

//-----------------------------------------------------
// WB bus ACK
//-----------------------------------------------------
always @ ( posedge CLK_I or posedge RST_I ) begin
	if( RST_I )
		TWI_ACK_O <= 1'b0 ;
	else if( TWI_STB_I && (~TWI_ACK_O) )
		TWI_ACK_O <= 1'b1 ;
	else
		TWI_ACK_O <= 1'b0 ;
end

wire i2cr_wr_en  = TWI_STB_I & TWI_WE_I  & ( TWI_ADR_I == `I2CR) & ~TWI_ACK_O ;
wire i2wd_wr_en  = TWI_STB_I & TWI_WE_I  & ( TWI_ADR_I == `I2WD) & ~TWI_ACK_O ;
wire pwm_wr_en   = TWI_STB_I & TWI_WE_I  & ( TWI_ADR_I == `PWMC) & ~TWI_ACK_O ;
wire wdg_wr_en   = TWI_STB_I & TWI_WE_I  & ( TWI_ADR_I == `WDG ) & ~TWI_ACK_O ;
wire sfta_wr_en  = TWI_STB_I & TWI_WE_I  & ( TWI_ADR_I == `SFTA) & ~TWI_ACK_O ;
wire time_wr_en  = TWI_STB_I & TWI_WE_I  & ( TWI_ADR_I == `TIME) & ~TWI_ACK_O ;
wire gpio_wr_en  = TWI_STB_I & TWI_WE_I  & ( TWI_ADR_I == `GPIO) & ~TWI_ACK_O ;
wire clko_wr_en  = TWI_STB_I & TWI_WE_I  & ( TWI_ADR_I == `CLKO) & ~TWI_ACK_O ;
wire sftb_wr_en  = TWI_STB_I & TWI_WE_I  & ( TWI_ADR_I == `SFTB) & ~TWI_ACK_O ;
wire sftc_wr_en  = TWI_STB_I & TWI_WE_I  & ( TWI_ADR_I == `SFTC) & ~TWI_ACK_O ;
wire sftd_wr_en  = TWI_STB_I & TWI_WE_I  & ( TWI_ADR_I == `SFTD) & ~TWI_ACK_O ;

wire i2cr_rd_en  = TWI_STB_I & ~TWI_WE_I  & ( TWI_ADR_I == `I2CR) & ~TWI_ACK_O ;
wire i2rd_rd_en  = TWI_STB_I & ~TWI_WE_I  & ( TWI_ADR_I == `I2RD) & ~TWI_ACK_O ;
wire wdg_rd_en   = TWI_STB_I & ~TWI_WE_I  & ( TWI_ADR_I == `WDG ) & ~TWI_ACK_O ;
wire sfta_rd_en  = TWI_STB_I & ~TWI_WE_I  & ( TWI_ADR_I == `SFTA) & ~TWI_ACK_O ;
wire fan0_rd_en  = TWI_STB_I & ~TWI_WE_I  & ( TWI_ADR_I == `FAN0) & ~TWI_ACK_O ;
wire fan1_rd_en  = TWI_STB_I & ~TWI_WE_I  & ( TWI_ADR_I == `FAN1) & ~TWI_ACK_O ;
wire time_rd_en  = TWI_STB_I & ~TWI_WE_I  & ( TWI_ADR_I == `TIME) & ~TWI_ACK_O ;
wire gpio_rd_en  = TWI_STB_I & ~TWI_WE_I  & ( TWI_ADR_I == `GPIO) & ~TWI_ACK_O ;
wire clko_rd_en  = TWI_STB_I & ~TWI_WE_I  & ( TWI_ADR_I == `CLKO) & ~TWI_ACK_O ;
wire sftb_rd_en  = TWI_STB_I & ~TWI_WE_I  & ( TWI_ADR_I == `SFTB) & ~TWI_ACK_O ;
wire sftc_rd_en  = TWI_STB_I & ~TWI_WE_I  & ( TWI_ADR_I == `SFTC) & ~TWI_ACK_O ;
wire sftd_rd_en  = TWI_STB_I & ~TWI_WE_I  & ( TWI_ADR_I == `SFTD) & ~TWI_ACK_O ;

//-----------------------------------------------------
// PWM
//-----------------------------------------------------
reg [9:0] reg_pwm ;
reg [9:0] pwm_cnt ;
always @ ( posedge CLK_I or posedge RST_I ) begin
	if( RST_I )
		reg_pwm <= 10'h00 ;
	else if( pwm_wr_en )
		reg_pwm <= TWI_DAT_I[9:0] ;
end

always @ ( posedge CLK_I or posedge RST_I ) begin
	if( RST_I )
		pwm_cnt <= 10'b0 ;
	else
		pwm_cnt <= pwm_cnt + 10'b1 ;
end

reg	  reg_pwm_update_ind ;
reg [9:0] reg_pwm_update ;
always @ ( posedge CLK_I or posedge RST_I ) begin
	if( RST_I )
		reg_pwm_update_ind  <= 1'b0 ;
	else if(pwm_cnt==10'd0)
		reg_pwm_update_ind  <= 1'b0 ;
	else if(pwm_wr_en && (|pwm_cnt))
		reg_pwm_update_ind  <= 1'b1 ;
end

always @ ( posedge CLK_I or posedge RST_I ) begin
	if( RST_I )
		reg_pwm_update	<= 10'd0 ;
	else if((pwm_cnt==10'd0) && (pwm_wr_en==0))
		reg_pwm_update	<= reg_pwm ;
end

assign PWM = (pwm_cnt < reg_pwm_update)&&(pwm_cnt > 10'd10);

//-----------------------------------------------------
// WDG
//-----------------------------------------------------
reg wdg_en ;
reg [30:0] wdg_cnt ;
reg [7:0] WATCH_DOG_f;
wire WATCH_DOG_tmp;

always @ (posedge CLK_I or posedge RST_I) begin
	if(RST_I)
		WATCH_DOG_f <= 8'b0;
	else
		WATCH_DOG_f <= {WATCH_DOG_f[6:0], WATCH_DOG_tmp};
end

always @ ( posedge CLK_I or posedge RST_I) begin
	if(RST_I)
		wdg_en <= 1'b0;
	else if(WATCH_DOG_tmp)
		wdg_en <= 1'b0 ;
	else if( wdg_wr_en )
		wdg_en <= TWI_DAT_I[0] ;
end

always @ ( posedge CLK_I or posedge RST_I) begin
	if(RST_I)
		wdg_cnt <= 32'b0;
	else if(wdg_wr_en && TWI_DAT_I[0])
		wdg_cnt <= TWI_DAT_I[31:1] ;
	else if( |wdg_cnt )
		wdg_cnt <= wdg_cnt - 31'b1 ;
end

assign WATCH_DOG_tmp = wdg_en && ~|wdg_cnt;
assign WATCH_DOG = |WATCH_DOG_f;

//-----------------------------------------------------
// SHIFT A
//-----------------------------------------------------
/*
00: Set master reset
01: Shift register
10: Storage register
11: Output Enable
*/

wire sfta_done ;
reg sfta_done_r ;
wire [31:0] reg_sfta;
assign reg_sfta = {28'b0,sfta_done_r,SFT_OE_N[0],2'b0} ;
always @ ( posedge CLK_I or posedge RST_I ) begin
	if( RST_I )
		sfta_done_r <= 1'b0 ;
	else if( sfta_wr_en )
		sfta_done_r <= 1'b0 ;
	else if( sfta_done )
		sfta_done_r <= 1'b1 ;
end

shift u_shift_a(
/*input       */ .clk      (CLK_I          ),
/*input       */ .rst      (RST_I          ),
/*input       */ .vld      (sfta_wr_en     ),
/*input  [1:0]*/ .cmd      (TWI_DAT_I[1:0] ),
/*input       */ .cmd_oen  (TWI_DAT_I[2]   ),
/*input  [7:0]*/ .din      (TWI_DAT_I[15:8]),
/*output      */ .done     (sfta_done      ),

/*output      */ .sft_shcp (SFT_SHCP[0]    ),
/*output      */ .sft_ds   (SFT_DS[0]      ),
/*output      */ .sft_stcp (SFT_STCP[0]    ),
/*output      */ .sft_mr_n (SFT_MR_N[0]    ),
/*output      */ .sft_oe_n (SFT_OE_N[0]    )
);

//-----------------------------------------------------
// SHIFT B
//-----------------------------------------------------
wire sftb_done ;
reg sftb_done_r ;
wire [31:0] reg_sftb;
assign reg_sftb = {28'b0,sftb_done_r,SFT_OE_N[1],2'b0} ;
always @ ( posedge CLK_I or posedge RST_I ) begin
	if( RST_I )
		sftb_done_r <= 1'b0 ;
	else if( sftb_wr_en )
		sftb_done_r <= 1'b0 ;
	else if( sftb_done )
		sftb_done_r <= 1'b1 ;
end

shift u_shift_b(
/*input       */ .clk      (CLK_I          ),
/*input       */ .rst      (RST_I          ),
/*input       */ .vld      (sftb_wr_en     ),
/*input  [1:0]*/ .cmd      (TWI_DAT_I[1:0] ),
/*input       */ .cmd_oen  (TWI_DAT_I[2]   ),
/*input  [7:0]*/ .din      (TWI_DAT_I[15:8]),
/*output      */ .done     (sftb_done      ),

/*output      */ .sft_shcp (SFT_SHCP[1]    ),
/*output      */ .sft_ds   (SFT_DS[1]      ),
/*output      */ .sft_stcp (SFT_STCP[1]    ),
/*output      */ .sft_mr_n (SFT_MR_N[1]    ),
/*output      */ .sft_oe_n (SFT_OE_N[1]    )
);

//-----------------------------------------------------
// SHIFT C
//-----------------------------------------------------
wire sftc_done ;
reg sftc_done_r ;
wire [31:0] reg_sftc;
assign reg_sftc = {28'b0,sftc_done_r,SFT_OE_N[2],2'b0} ;
always @ ( posedge CLK_I or posedge RST_I ) begin
	if( RST_I )
		sftc_done_r <= 1'b0 ;
	else if( sftc_wr_en )
		sftc_done_r <= 1'b0 ;
	else if( sftc_done )
		sftc_done_r <= 1'b1 ;
end

shift u_shift_c(
/*input       */ .clk      (CLK_I          ),
/*input       */ .rst      (RST_I          ),
/*input       */ .vld      (sftc_wr_en     ),
/*input  [1:0]*/ .cmd      (TWI_DAT_I[1:0] ),
/*input       */ .cmd_oen  (TWI_DAT_I[2]   ),
/*input  [7:0]*/ .din      (TWI_DAT_I[15:8]),
/*output      */ .done     (sftc_done      ),

/*output      */ .sft_shcp (SFT_SHCP[2]    ),
/*output      */ .sft_ds   (SFT_DS[2]      ),
/*output      */ .sft_stcp (SFT_STCP[2]    ),
/*output      */ .sft_mr_n (SFT_MR_N[2]    ),
/*output      */ .sft_oe_n (SFT_OE_N[2]    )
);

//-----------------------------------------------------
// SHIFT D
//-----------------------------------------------------
wire sftd_done ;
reg sftd_done_r ;
wire [31:0] reg_sftd;

assign reg_sftd = {28'b0,sftd_done_r,SFT_OE_N[3],2'b0} ;
always @ ( posedge CLK_I or posedge RST_I ) begin
	if( RST_I )
		sftd_done_r <= 1'b0 ;
	else if( sftd_wr_en )
		sftd_done_r <= 1'b0 ;
	else if( sftd_done )
		sftd_done_r <= 1'b1 ;
end

shift u_shift_d(
/*input       */ .clk      (CLK_I          ),
/*input       */ .rst      (RST_I          ),
/*input       */ .vld      (sftd_wr_en     ),
/*input  [1:0]*/ .cmd      (TWI_DAT_I[1:0] ),
/*input       */ .cmd_oen  (TWI_DAT_I[2]   ),
/*input  [7:0]*/ .din      (TWI_DAT_I[15:8]),
/*output      */ .done     (sftd_done      ),

/*output      */ .sft_shcp (SFT_SHCP[3]    ),
/*output      */ .sft_ds   (SFT_DS[3]      ),
/*output      */ .sft_stcp (SFT_STCP[3]    ),
/*output      */ .sft_mr_n (SFT_MR_N[3]    ),
/*output      */ .sft_oe_n (SFT_OE_N[3]    )
);

//-----------------------------------------------------
// fan speed
//-----------------------------------------------------
reg [26:0] sec_cnt ;//1m
always @ ( posedge CLK_I or posedge RST_I ) begin
	if( RST_I )
		sec_cnt <= 27'b0 ;
	else if( sec_cnt == `MM_CLK_1S_CNT )
		sec_cnt <= 27'b0 ;
	else
		sec_cnt <= 27'b1 + sec_cnt ;
end

//fan0
reg [7:0] fan0_r;
always @ (posedge CLK_I) begin
	fan0_r <= {fan0_r[6:0], FAN_IN0};
end
reg is_fan0;
always @ (posedge CLK_I) begin
	is_fan0 <= &fan0_r;
end

reg [26:0] fan_cnt0 ;
reg [26:0] reg_fan0 ;
reg [2:0] fan0_f ;
wire fan0_neg = ~fan0_f[1] && fan0_f[2] ;
always @ ( posedge CLK_I or posedge RST_I) begin
	if(RST_I)
		fan0_f <= 3'b0;
	else
		fan0_f <= {fan0_f[1:0],is_fan0} ;
end

always @ ( posedge CLK_I or posedge RST_I ) begin
	if( RST_I )
		fan_cnt0 <= 27'b0 ;
	else if( sec_cnt == `MM_CLK_1S_CNT ) begin
		fan_cnt0 <= 27'b0 ;
		reg_fan0 <= fan_cnt0 ;
	end else if( fan0_neg )
		fan_cnt0 <= fan_cnt0 + 27'b1 ;
end

//fan1
reg [26:0] fan_cnt1 ;
reg [26:0] reg_fan1 ;
reg [2:0] fan1_f ;
wire fan1_neg = ~fan1_f[1] && fan1_f[2] ;
always @ ( posedge CLK_I or posedge RST_I) begin
	if(RST_I)
		fan1_f <= 3'b0;
	else
		fan1_f <= {fan1_f[1:0],FAN_IN1} ;
end

always @ ( posedge CLK_I or posedge RST_I ) begin
	if( RST_I )
		fan_cnt1 <= 27'b0 ;
	else if( sec_cnt == `MM_CLK_1S_CNT ) begin
		fan_cnt1 <= 27'b0 ;
		reg_fan1 <= fan_cnt1 ;
	end else if( fan1_neg )
		fan_cnt1 <= fan_cnt1 + 27'b1 ;
end


//-----------------------------------------------------
// timer
//-----------------------------------------------------
//1s 2faf080 SEC
reg [26:0] tim_cnt ;
reg [5:0] sec_cnt0 ;
reg [5:0] sec_cnt0_f ;
reg tim_done0 ;
reg [5:0] sec_cnt1 ;
reg [5:0] sec_cnt1_f ;
reg tim_done1 ;
reg tim_mask0 ;
reg tim_mask1 ;
wire [31:0] reg_tim = {7'b0,tim_done1,sec_cnt1,tim_mask1,1'b0,7'b0,tim_done0,sec_cnt0,tim_mask0,1'b0} ;
always @ ( posedge CLK_I or posedge RST_I) begin
	if(RST_I)
		tim_cnt <= 27'b0;
	else if( tim_cnt == `MM_CLK_1S_CNT )
		tim_cnt <= 27'b0 ;
	else
		tim_cnt <= 27'b1 + tim_cnt ;
end

always @ ( posedge CLK_I or posedge RST_I ) begin
	if( RST_I )
		sec_cnt0 <= 6'b0 ;
	else if( time_wr_en && TWI_DAT_I[0] )
		sec_cnt0 <= TWI_DAT_I[7:2] ;
	else if( |sec_cnt0 && tim_cnt == `MM_CLK_1S_CNT )
		sec_cnt0 <= sec_cnt0 - 6'b1 ;
end

always @ ( posedge CLK_I or posedge RST_I) begin
	if(RST_I)
		sec_cnt0_f <= 6'b0;
	else
		sec_cnt0_f <= sec_cnt0 ;
end

always @ ( posedge CLK_I or posedge RST_I ) begin
	if( RST_I )
		tim_mask0 <= 1'b1 ;
	else if( time_wr_en )
		tim_mask0 <= TWI_DAT_I[1] ;
end

always @ ( posedge CLK_I or posedge RST_I ) begin
	if( RST_I )
		sec_cnt1 <= 6'b0 ;
	else if( time_wr_en && TWI_DAT_I[16] )
		sec_cnt1 <= TWI_DAT_I[23:18] ;
	else if( |sec_cnt1 && tim_cnt == `MM_CLK_1S_CNT )
		sec_cnt1 <= sec_cnt1 - 6'b1 ;
end

always @ ( posedge CLK_I or posedge RST_I) begin
	if(RST_I)
		sec_cnt1_f <= 6'b0;
	else
		sec_cnt1_f <= sec_cnt1 ;
end

always @ ( posedge CLK_I or posedge RST_I ) begin
	if( RST_I )
		tim_done0 <= 1'b0 ;
	else if( sec_cnt0 == 0 && sec_cnt0_f == 1 )
		tim_done0 <= 1'b1 ;
	else if( time_wr_en && TWI_DAT_I[8] )
		tim_done0 <= 1'b0 ;
end

always @ ( posedge CLK_I or posedge RST_I ) begin
	if( RST_I )
		tim_done1 <= 1'b0 ;
	else if( sec_cnt1 == 0 && sec_cnt1_f == 1 )
		tim_done1 <= 1'b1 ;
	else if( time_wr_en && TWI_DAT_I[24] )
		tim_done1 <= 1'b0 ;
end


always @ ( posedge CLK_I or posedge RST_I ) begin
	if( RST_I )
		tim_mask1 <= 1'b1 ;
	else if( time_wr_en )
		tim_mask1 <= TWI_DAT_I[17] ;
end

assign TIME0_INT = ~tim_mask0 && tim_done0 ;
assign TIME1_INT = ~tim_mask1 && tim_done1 ;
//-----------------------------------------------------
// GPIO
//-----------------------------------------------------
reg [15:0] reg_gout ;
reg [15:0] reg_gin  ;
wire [31:0] reg_gpio = {reg_gin, reg_gout} ;
always @ ( posedge CLK_I or posedge RST_I ) begin
	if( RST_I )
		reg_gout <= 16'h8800;//for vid oe_n
	else if( gpio_wr_en )
		reg_gout <= TWI_DAT_I[15:0];
end
assign GPIO_OUT = reg_gout ;
always @ ( posedge CLK_I or posedge RST_I ) begin
	if(RST_I)
		reg_gin <= 16'b0;
	else
		reg_gin <= GPIO_IN ;
end


//-----------------------------------------------------
// CLK OUT 25M Enable
//-----------------------------------------------------
reg [7:0] reg_clk25m_on;
assign clk25m_on = reg_clk25m_on;
always @ ( posedge CLK_I or posedge RST_I ) begin
	if( RST_I )
		reg_clk25m_on <= 8'b0;
	else if(clko_wr_en)
		reg_clk25m_on <= TWI_DAT_I[7:0];
end

//-----------------------------------------------------
// read
//-----------------------------------------------------
wire [7:0] reg_i2cr ;
wire [7:0] reg_i2rd ;
reg [31:0] bus_data_o;

assign TWI_DAT_O = bus_data_o;
always @ (posedge CLK_I) begin
	bus_data_o <=   i2cr_rd_en ? {24'b0,reg_i2cr}:
			wdg_rd_en  ? {wdg_cnt,wdg_en}:
			sfta_rd_en ? reg_sfta        :
			fan0_rd_en ? {5'b0,reg_fan0} :
			fan1_rd_en ? {5'b0,reg_fan1} :
			time_rd_en ? reg_tim         :
			gpio_rd_en ? reg_gpio        :
			clko_rd_en ? reg_clk25m_on   :
			sftb_rd_en ? reg_sftb        :
			sftc_rd_en ? reg_sftc        :
			sftd_rd_en ? reg_sftd        : {24'b0,reg_i2rd} ;
end

twi_core twi_core (
/*input       */ .clk          (CLK_I                ) ,
/*input       */ .rst          (RST_I                ) ,
/*input       */ .wr           (i2cr_wr_en|i2wd_wr_en) , //we
/*input  [7:0]*/ .data_in      (TWI_DAT_I[7:0]       ) ,//dat1
/*input  [7:0]*/ .wr_addr      ({2'b0,TWI_ADR_I}     ) ,//adr1
/*output [7:0]*/ .i2cr         (reg_i2cr             ) ,
/*output [7:0]*/ .i2rd         (reg_i2rd             ) ,
/*output      */ .twi_scl_o    (TWI_SCL_O            ) ,
/*input       */ .twi_sda_i    (TWI_SDA_I            ) ,
/*output      */ .twi_sda_oen  (TWI_SDA_OEN          )
);

endmodule
