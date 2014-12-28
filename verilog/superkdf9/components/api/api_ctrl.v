`include "api_define.v"

module api_ctrl(
input                 clk               ,
input                 rst               ,

output [2:0]          reg_state         ,
input  [27:0]         reg_timeout       ,
input  [7:0]          reg_sck           ,
input  [5:0]          reg_ch_num        ,
input  [7:0]          reg_word_num      ,

input                 tx_fifo_empty     ,
output                tx_fifo_rd_en     ,
input  [31:0]         tx_fifo_dout      ,

output                rx_fifo_wr_en     ,
output [31:0]         rx_fifo_din       ,
input  [9:0]          rx_fifo_data_count,

output reg [`API_NUM-1:0] load          ,
output                sck               ,
output                mosi              ,
input  [`API_NUM-1:0] miso              ,

output                led_get_nonce_l   ,
output                led_get_nonce_h    
);
parameter RX_FIFO_DEPTH = 512;//words
parameter WORK_LEN = 736/32;//words
parameter RX_BLOCK_LEN = 11;//words
parameter MAX_CHIP_IN_CH = 5;//words

parameter WAIT = 2'd0;
parameter WORK = 2'd1;
parameter NOP  = 2'd2;

reg [1:0] cur_state;
reg [1:0] nxt_state;
wire timeout_busy;
wire timer_start = cur_state != WORK && nxt_state == WORK;
reg [5:0] ch_cnt;
reg [7:0] word_cnt;
reg mosi_vld;
wire miso_vld;
reg [3:0] load_nop_cnt;
assign tx_fifo_rd_en = mosi_vld && cur_state == WORK;
wire rx_fifo_full = (RX_FIFO_DEPTH - rx_fifo_data_count) < (RX_BLOCK_LEN * MAX_CHIP_IN_CH);
wire [31:0] miso_dat;
assign reg_state = {1'b0, cur_state};

always @ (posedge clk) begin
	if(rst)
		cur_state <= WAIT;
	else
		cur_state <= nxt_state;
end

always @ (*) begin
	nxt_state = cur_state;
	case(cur_state)
	WAIT:   if(~timeout_busy && ~tx_fifo_empty && ~rx_fifo_full)
			nxt_state = WORK;
	WORK:	if(word_cnt == reg_word_num)
			nxt_state = NOP;
	NOP :   if(&load_nop_cnt)
			nxt_state = WAIT;
	endcase
end
always @ (posedge clk) begin
	if(rst)
		word_cnt <= 8'b0;
	else if(cur_state != WORK && nxt_state == WORK)
		word_cnt <= 8'b0;
	else if(cur_state == WORK && word_cnt != reg_word_num && miso_vld)
		word_cnt <= word_cnt + 8'b1;
end

always @ (posedge clk) begin
	if(rst)
		mosi_vld <= 1'b0;
	else if(cur_state != WORK && nxt_state == WORK)
		mosi_vld <= 1'b1;
	else if(cur_state == WORK && miso_vld && word_cnt < (reg_word_num - 1))
		mosi_vld <= 1'b1;
	else
		mosi_vld <= 1'b0;
end

always @ (posedge clk) begin
	if(rst)
		load_nop_cnt <= 4'b0;
	else if(cur_state != NOP && nxt_state == NOP)
		load_nop_cnt <= 4'b1;
	else if(~&load_nop_cnt && cur_state == NOP)
		load_nop_cnt <= load_nop_cnt + 4'b1;
	else if(cur_state == NOP && nxt_state != NOP)
		load_nop_cnt <= 4'b0;
end

reg [4:0] work_cnt;
always @ (posedge clk) begin
	if(rst)
		work_cnt <= 5'b0;
	else if(cur_state != WORK && nxt_state == WORK)
		work_cnt <= 5'b0;
	else if(cur_state == WORK && word_cnt != reg_word_num && miso_vld && work_cnt != 5'd22)
		work_cnt <= work_cnt + 5'b1;
	else if(cur_state == WORK && word_cnt != reg_word_num && miso_vld && work_cnt == 5'd22)
		work_cnt <= 5'b0;
end


assign rx_fifo_wr_en = miso_vld && (work_cnt < RX_BLOCK_LEN) && (cur_state == WORK);
wire [3:0] miner_id =   load == `API_NUM'b1111111110 ? 4'd0 :
			load == `API_NUM'b1111111101 ? 4'd1 :
			load == `API_NUM'b1111111011 ? 4'd2 :
			load == `API_NUM'b1111110111 ? 4'd3 :
			load == `API_NUM'b1111101111 ? 4'd4 :
			load == `API_NUM'b1111011111 ? 4'd5 :
			load == `API_NUM'b1110111111 ? 4'd6 :
			load == `API_NUM'b1101111111 ? 4'd7 :
			load == `API_NUM'b1011111111 ? 4'd8 :
			load == `API_NUM'b0111111111 ? 4'd9 : 4'd10;

assign rx_fifo_din = (work_cnt == (RX_BLOCK_LEN-1)) ? {miso_dat[31:16], 8'h12, 4'b0, miner_id} : miso_dat;

reg led_get_nonce_l_r;
always @ (posedge clk) begin
	if(rst)
		led_get_nonce_l_r <= 1'b0;
	else if(rx_fifo_wr_en && (work_cnt == 2) && (miner_id <= 4))
		led_get_nonce_l_r <= 1'b1;
	else if(led_get_nonce_l)
		led_get_nonce_l_r <= 1'b0;
end

reg led_get_nonce_h_r;
always @ (posedge clk) begin
	if(rst)
		led_get_nonce_h_r <= 1'b0;
	else if(rx_fifo_wr_en && (work_cnt == 2) && (miner_id > 4))
		led_get_nonce_h_r <= 1'b1;
	else if(led_get_nonce_h)
		led_get_nonce_h_r <= 1'b0;
end

assign led_get_nonce_l = led_get_nonce_l_r && rx_fifo_wr_en && (work_cnt == RX_BLOCK_LEN - 2) && (miso_dat == 32'hbeafbeaf) && (miner_id <= 4);
assign led_get_nonce_h = led_get_nonce_h_r && rx_fifo_wr_en && (work_cnt == RX_BLOCK_LEN - 2) && (miso_dat == 32'hbeafbeaf) && (miner_id > 4);

always @ (posedge clk) begin
	if(rst)
		load <= {`API_NUM{1'b1}} ^ `API_NUM'b1;
	else if(cur_state == NOP && nxt_state == WAIT)
		load <= {load[`API_NUM-2:0], load[`API_NUM-1]};
end

always @ (posedge clk) begin
	if(rst)
		ch_cnt <= 6'b0;
	else if(cur_state == WAIT && nxt_state == WORK && ch_cnt == reg_ch_num)
		ch_cnt <= 6'b0;
	else if(cur_state != WORK && nxt_state == WORK)
		ch_cnt <= 6'b1 + ch_cnt;
end

wire miso_w = &(miso | load);

api_timer api_timer(
/*input          */ .clk         (clk         ),
/*input          */ .rst         (rst         ),

/*input  [27:0]  */ .reg_timeout (reg_timeout ),
/*input          */ .start       (timer_start ),
/*output         */ .timeout_busy(timeout_busy)
);

api_phy api_phy(
/*input          */ .clk         (clk         ),
/*output         */ .rst         (rst         ),

/*input  [7:0]   */ .reg_sck     (reg_sck     ),

/*input          */ .mosi_vld    (mosi_vld    ),
/*input  [31:0]  */ .mosi_dat    (tx_fifo_dout),

/*output         */ .miso_vld    (miso_vld    ),
/*output [31:0]  */ .miso_dat    (miso_dat    ),

/*output         */ .sck         (sck         ),
/*output         */ .mosi        (mosi        ),
/*input          */ .miso        (miso_w      )
);

//-----------------------------------------------
//ila
//-----------------------------------------------
/*
reg [31:0] tx_cnt;
reg [31:0] sec;
always @ (posedge clk) begin
	if(sec != 1000000000)
		sec <= sec + 1;
	else
		sec <= 0;
end

always @ (posedge clk) begin
	if(sec == 0)
		tx_cnt <= 0;
	else if(cur_state == WAIT && ~timeout_busy && tx_fifo_empty && ~rx_fifo_full)
		tx_cnt <= tx_cnt + 1;
end

wire [35:0] icon_ctrl_0;
wire [255:0] trig0 = {
sec,//69:38
cur_state,//37:36
nxt_state,//35:34
rx_fifo_full,//33
tx_fifo_empty,//32
tx_cnt[31:0] //
} ;
icon icon_test(.CONTROL0(icon_ctrl_0));
ila ila_test(.CONTROL(icon_ctrl_0), .CLK(clk), .TRIG0(trig0)
);
*/
endmodule
