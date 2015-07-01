`include "api_define.v"
module api_slave(
// system clock and reset
input             clk                ,
input             rst                ,

// wishbone interface signals
input             API_CYC_I          ,//NC
input             API_STB_I          ,
input             API_WE_I           ,
input             API_LOCK_I         ,//NC
input  [2:0]      API_CTI_I          ,//NC
input  [1:0]      API_BTE_I          ,//NC
input  [5:0]      API_ADR_I          ,
input  [31:0]     API_DAT_I          ,
input  [3:0]      API_SEL_I          ,//NC
output reg        API_ACK_O          ,
output            API_ERR_O          ,//const 0
output            API_RTY_O          ,//const 0
output reg [31:0] API_DAT_O          ,

output reg        txfifo_push        ,
output reg [31:0] txfifo_din         ,

input  [9 :0]     rxcnt              ,
input             rxempty            ,
input  [10:0]     txcnt              ,
output            reg_flush          ,
input             txfull             ,

input  [2:0]      reg_state          ,
output reg [27:0] reg_timeout        ,
output reg [7:0]  reg_sck            ,
output reg [5:0]  reg_ch_num         ,
output reg [8:0]  reg_word_num       ,

input             rx_fifo_wr_en      ,
input  [31:0]     rx_fifo_din        ,
input  [3:0]      miner_id           ,
input  [4:0]      work_cnt           ,

output            rxfifo_pop         ,
input  [31:0]     rxfifo_dout        ,

output reg        reg_pllf_rst       ,
output reg[103:0] reg_pllf_data      ,
output reg        reg_pllf_wr_en     ,
input             reg_pllf_full      ,
input             reg_pllf_empty     ,
input [6 : 0]     reg_pllf_data_count 
);

parameter API_TXFIFO  = 6'h00;
parameter API_RXFIFO  = 6'h04;
parameter API_STATE   = 6'h08;
parameter API_TIMEOUT = 6'h0c;
parameter API_SCK     = 6'h10;
parameter API_RAM     = 6'h14;
parameter API_LM      = 6'h18;//local work
parameter API_PLLA    = 6'h1c;
parameter API_PLLC0   = 6'h20;
parameter API_PLLC1   = 6'h24;
parameter API_PLLC2   = 6'h28;
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

wire api_ram_wr_en = API_STB_I & API_WE_I  & ( API_ADR_I == API_RAM ) & ~API_ACK_O ;
wire api_ram_rd_en = API_STB_I & ~API_WE_I  & ( API_ADR_I == API_RAM ) & ~API_ACK_O ;

wire api_lw_wr_en = API_STB_I & API_WE_I  & ( API_ADR_I == API_LM ) & ~API_ACK_O ;
wire api_lw_rd_en = API_STB_I & ~API_WE_I  & ( API_ADR_I == API_LM ) & ~API_ACK_O ;

wire api_plla_wr_en = API_STB_I & API_WE_I  & ( API_ADR_I == API_PLLA ) & ~API_ACK_O ;
wire api_plla_rd_en = API_STB_I & API_WE_I  & ( API_ADR_I == API_PLLA ) & ~API_ACK_O ;

wire api_pllc0_wr_en = API_STB_I & API_WE_I  & ( API_ADR_I == API_PLLC0 ) & ~API_ACK_O ;
wire api_pllc1_wr_en = API_STB_I & API_WE_I  & ( API_ADR_I == API_PLLC1 ) & ~API_ACK_O ;
wire api_pllc2_wr_en = API_STB_I & API_WE_I  & ( API_ADR_I == API_PLLC2 ) & ~API_ACK_O ;
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
wire [31:0] rd_state = {2'h0, rxcnt[9:0], 3'b0, rxempty,
			reg_state, txcnt[10:0], reg_flush, txfull};

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
	if( api_sck_wr_en     ) reg_word_num[8:0] <= API_DAT_I[31:23];
end

//-----------------------------------------------------
// PLL
//-----------------------------------------------------
always @ (posedge clk) begin
	if(api_plla_wr_en) reg_pllf_data[7:0]  <= API_DAT_I[7:0];
	if(api_plla_wr_en) reg_pllf_rst <= API_DAT_I[23]; else reg_pllf_rst <= 1'b0;
	if(api_pllc0_wr_en) reg_pllf_data[39:8] <= API_DAT_I[31:0];
	if(api_pllc1_wr_en) reg_pllf_data[71:40] <= API_DAT_I[31:0];
	if(api_pllc2_wr_en) reg_pllf_data[103:72] <= API_DAT_I[31:0];
	if(api_pllc2_wr_en) reg_pllf_wr_en <= 1'b1; else reg_pllf_wr_en <= 1'b0;
end

//-----------------------------------------------------
// RAM
//-----------------------------------------------------

reg [8:0] tram_addr;                                                                                              
wire [31:0] tram_dout;

always @ (posedge clk) begin                                                                                    
        if(api_ram_wr_en)
                tram_addr <= API_DAT_I[8:0];                                                                    
end

test_data test_data(
/*input          */ .clka (clk),
/*input  [8 : 0] */ .addra(tram_addr),
/*output [31 : 0]*/ .douta(tram_dout)
);

//-----------------------------------------------------
// Local Work
//-----------------------------------------------------
reg [23:0] lw0;
reg [23:0] lw1;
reg [23:0] lw2;
reg [23:0] lw3;
reg [23:0] lw4;
reg [23:0] lw5;
reg [23:0] lw6;
reg [23:0] lw7;
reg [23:0] lw8;
reg [23:0] lw9;

reg [23:0] reg_lw;
always @ (posedge clk) begin
	if(api_lw_wr_en)
		reg_lw <= API_DAT_I[3:0] == 0 ? lw0 :
			  API_DAT_I[3:0] == 1 ? lw1 :
			  API_DAT_I[3:0] == 2 ? lw2 :
			  API_DAT_I[3:0] == 3 ? lw3 :
			  API_DAT_I[3:0] == 4 ? lw4 :
			  API_DAT_I[3:0] == 5 ? lw5 :
			  API_DAT_I[3:0] == 6 ? lw6 :
			  API_DAT_I[3:0] == 7 ? lw7 :
			  API_DAT_I[3:0] == 8 ? lw8 : lw9;
end

reg [31:0] rx_fifo_din_r;
wire lw_vld_data = rx_fifo_wr_en && (work_cnt > 1) && (work_cnt < 10);
wire lw_vld = lw_vld_data && (rx_fifo_din != 32'hbeafbeaf) && (rx_fifo_din != rx_fifo_din_r) && (|rx_fifo_din) && (|(~rx_fifo_din));
always @ (posedge clk) begin
	if(lw_vld_data)
		rx_fifo_din_r <= rx_fifo_din;
end

wire overflow = &lw0 || &lw1 || &lw2 || &lw3 || &lw4 || &lw5 || &lw6 || &lw7 || &lw8 || &lw9;

always @ (posedge clk) begin
        if((api_lw_wr_en && API_DAT_I[3:0] == 0) || overflow) lw0 <= 0;
	else if(lw_vld && miner_id == 4'd0) lw0 <= lw0 + 24'b1;

        if((api_lw_wr_en && API_DAT_I[3:0] == 1) || overflow) lw1 <= 0;
        else if(lw_vld && miner_id == 4'd1) lw1 <= lw1 + 24'b1;

        if((api_lw_wr_en && API_DAT_I[3:0] == 2) || overflow) lw2 <= 0;
        else if(lw_vld && miner_id == 4'd2) lw2 <= lw2 + 24'b1;

        if((api_lw_wr_en && API_DAT_I[3:0] == 3) || overflow) lw3 <= 0;
        else if(lw_vld && miner_id == 4'd3) lw3 <= lw3 + 24'b1;

        if((api_lw_wr_en && API_DAT_I[3:0] == 4) || overflow) lw4 <= 0;
        else if(lw_vld && miner_id == 4'd4) lw4 <= lw4 + 24'b1;

        if((api_lw_wr_en && API_DAT_I[3:0] == 5) || overflow) lw5 <= 0;
        else if(lw_vld && miner_id == 4'd5) lw5 <= lw5 + 24'b1;

        if((api_lw_wr_en && API_DAT_I[3:0] == 6) || overflow) lw6 <= 0;
        else if(lw_vld && miner_id == 4'd6) lw6 <= lw6 + 24'b1;

        if((api_lw_wr_en && API_DAT_I[3:0] == 7) || overflow) lw7 <= 0;
        else if(lw_vld && miner_id == 4'd7) lw7 <= lw7 + 24'b1;

        if((api_lw_wr_en && API_DAT_I[3:0] == 8) || overflow) lw8 <= 0;
        else if(lw_vld && miner_id == 4'd8) lw8 <= lw8 + 24'b1;

        if((api_lw_wr_en && API_DAT_I[3:0] == 9) || overflow) lw9 <= 0;
        else if(lw_vld && miner_id == 4'd9) lw9 <= lw9 + 24'b1;
end

//-----------------------------------------------------
// WB read
//-----------------------------------------------------
assign rxfifo_pop = api_rxfifo_rd_en ;

always @ ( posedge clk ) begin
	case( 1'b1 )
		api_state_rd_en  : API_DAT_O <= rd_state  ;
		api_rxfifo_rd_en : API_DAT_O <= rxempty ? 32'h12345678 : rd_rxfifo ;
		api_timeout_rd_en: API_DAT_O <= {4'b0, reg_timeout[27:0]};
		api_sck_rd_en    : API_DAT_O <= {reg_word_num[8:0], 1'b0,reg_ch_num[5:0], 8'h0, reg_sck[7:0]};
		api_ram_rd_en    : API_DAT_O <= tram_dout;
		api_lw_rd_en     : API_DAT_O <= {8'b0, reg_lw};
		api_plla_rd_en   : API_DAT_O <= {9'b0, reg_pllf_empty, reg_pllf_full, 2'b0, reg_pllf_data_count,12'b0};
		default: API_DAT_O <= 32'hdeaddead ; 
	endcase
end


endmodule

module test_data(
input           clka ,
input  [8 : 0]  addra,
output [31 : 0] douta 
);

endmodule

