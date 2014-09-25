`include "api_define.v"
module api_slave(
// system clock and reset
input             clk         ,
input             rst         ,

// wishbone interface signals
input             API_CYC_I   ,//NC
input             API_STB_I   ,
input             API_WE_I    ,
input             API_LOCK_I  ,//NC
input  [2:0]      API_CTI_I   ,//NC
input  [1:0]      API_BTE_I   ,//NC
input  [5:0]      API_ADR_I   ,
input  [31:0]     API_DAT_I   ,
input  [3:0]      API_SEL_I   ,//NC
output reg        API_ACK_O   ,
output            API_ERR_O   ,//const 0
output            API_RTY_O   ,//const 0
output reg [31:0] API_DAT_O   ,

output reg        txfifo_push ,
output reg [31:0] txfifo_din  ,

input  [8 :0]     rxcnt       ,
input             rxempty     ,
input  [9 :0]     txcnt       ,
output            reg_flush   ,
input             txfull      ,

input  [2:0]      reg_state   ,
output reg [27:0] reg_timeout ,
output reg [7:0]  reg_sck     ,
output reg [5:0]  reg_ch_num  ,
output reg [7:0]  reg_word_num,

output            rxfifo_pop  ,
input  [31:0]     rxfifo_dout   
);

parameter API_TXFIFO  = 6'h00;
parameter API_RXFIFO  = 6'h04;
parameter API_STATE   = 6'h08;
parameter API_TIMEOUT = 6'h0c;
parameter API_SCK     = 6'h10;

//-----------------------------------------------------
// WB bus ACK
//-----------------------------------------------------
always @ ( posedge clk or posedge rst ) begin
        if( rst )
                API_ACK_O <= 1'b0 ;
        else if( API_STB_I && (~API_ACK_O) )
                API_ACK_O <= 1'b1 ;
        else 
                API_ACK_O <= 1'b0 ;
end

assign API_ERR_O = 1'b0;
assign API_RTY_O = 1'b0;
//-----------------------------------------------------
// ADDR MUX
//-----------------------------------------------------

wire api_txfifo_wr_en = API_STB_I & API_WE_I  & ( API_ADR_I == API_TXFIFO ) & ~API_ACK_O ;
wire api_txfifo_rd_en = API_STB_I & ~API_WE_I & ( API_ADR_I == API_TXFIFO ) & ~API_ACK_O ;

wire api_rxfifo_wr_en = API_STB_I & API_WE_I  & ( API_ADR_I == API_RXFIFO ) & ~API_ACK_O ;
wire api_rxfifo_rd_en = API_STB_I & ~API_WE_I & ( API_ADR_I == API_RXFIFO ) & ~API_ACK_O ;

wire api_state_wr_en = API_STB_I & API_WE_I  & ( API_ADR_I == API_STATE ) & ~API_ACK_O ;
wire api_state_rd_en = API_STB_I & ~API_WE_I & ( API_ADR_I == API_STATE ) & ~API_ACK_O ;

wire api_timeout_wr_en = API_STB_I & API_WE_I  & ( API_ADR_I == API_TIMEOUT ) & ~API_ACK_O ;
wire api_timeout_rd_en = API_STB_I & ~API_WE_I & ( API_ADR_I == API_TIMEOUT ) & ~API_ACK_O ;

wire api_sck_wr_en = API_STB_I & API_WE_I  & ( API_ADR_I == API_SCK ) & ~API_ACK_O ;
wire api_sck_rd_en = API_STB_I & ~API_WE_I & ( API_ADR_I == API_SCK ) & ~API_ACK_O ;

//-----------------------------------------------------
// Register.txfifo
//-----------------------------------------------------
always @ ( posedge clk ) begin
	txfifo_push <= api_txfifo_wr_en ;
	txfifo_din  <= API_DAT_I ;
end

//-----------------------------------------------------
// Register.state
//-----------------------------------------------------
reg [3:0] reg_flush_r ;
wire [31:0] rd_state = {3'h0, rxcnt[8:0], 3'b0, rxempty,
			reg_state, 1'b0, txcnt[9:0], reg_flush, txfull};

always @ ( posedge clk ) begin
	if( api_state_wr_en )
		reg_flush_r <= {3'b0,API_DAT_I[1]} ;
	else
		reg_flush_r <= reg_flush_r << 1 ;
end

assign reg_flush = |reg_flush_r ;

//-----------------------------------------------------
// Register.rxfifo
//-----------------------------------------------------
wire [31:0] rd_rxfifo = rxfifo_dout[31:0] ;

always @ ( posedge clk ) begin
	if( api_timeout_wr_en ) reg_timeout[27:0] <= API_DAT_I[27:0];
	if( api_sck_wr_en     ) reg_sck[7:0]      <= API_DAT_I[7:0];
	if( api_sck_wr_en     ) reg_ch_num[5:0]   <= API_DAT_I[21:16];
	if( api_sck_wr_en     ) reg_word_num[7:0] <= API_DAT_I[31:24];
end


//-----------------------------------------------------
// WB read
//-----------------------------------------------------
assign rxfifo_pop = api_rxfifo_rd_en ;

always @ ( posedge clk ) begin
	case( 1'b1 )
		api_state_rd_en  : API_DAT_O <= rd_state  ;
		api_rxfifo_rd_en : API_DAT_O <= rd_rxfifo ;
		api_timeout_rd_en: API_DAT_O <= {4'b0, reg_timeout[27:0]};
		api_sck_rd_en    : API_DAT_O <= {reg_word_num[7:0], 2'b0,reg_ch_num[5:0], 8'h0, reg_sck[7:0]};
		default: API_DAT_O <= 32'hdeaddead ; 
	endcase
end

endmodule

