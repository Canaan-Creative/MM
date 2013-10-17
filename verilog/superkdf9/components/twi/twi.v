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
    output         PWM          
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

wire i2cr_rd_en  = TWI_STB_I & ~TWI_WE_I  & ( TWI_ADR_I == `I2CR) & ~TWI_ACK_O ;
wire i2rd_rd_en  = TWI_STB_I & ~TWI_WE_I  & ( TWI_ADR_I == `I2RD) & ~TWI_ACK_O ;


//-----------------------------------------------------
// PWM
//-----------------------------------------------------
reg [7:0] reg_pwm ;
reg [7:0] pwm_cnt ;
always @ ( posedge CLK_I or posedge RST_I ) begin
	if( RST_I )
		reg_pwm <= 8'h00 ;
	else if( pwm_wr_en )
		reg_pwm <= TWI_DAT_I[7:0] ;
end

always @ ( posedge CLK_I or posedge RST_I ) begin
	if( RST_I )
		pwm_cnt <= 8'b0 ;
	else
		pwm_cnt <= pwm_cnt + 8'b1 ;
end

assign PWM = pwm_cnt >= reg_pwm ;

//-----------------------------------------------------
// read
//-----------------------------------------------------
reg i2cr_rd_en_r ;
wire [7:0] reg_i2cr ;
wire [7:0] reg_i2rd ;
always @ ( posedge CLK_I ) begin
	i2cr_rd_en_r <= i2cr_rd_en ;
end

assign TWI_DAT_O = i2cr_rd_en_r ? {24'b0,reg_i2cr} : {24'b0,reg_i2rd} ;

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

