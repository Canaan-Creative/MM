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
input  [8:0]          rx_fifo_data_count,

output reg [`API_NUM-1:0] load          ,
output                sck               ,
output                mosi              ,
input  [`API_NUM-1:0] miso
);
parameter RX_FIFO_DEPTH = 256;//words
parameter WORK_LEN = 736/32;//words
parameter RX_BLOCK_LEN = 4;//words
parameter MAX_CHIP_IN_CH = 5;//words

parameter IDLE = 2'd0;
parameter WORK = 2'd1;
parameter NOP  = 2'd2;
parameter DONE = 2'd3;

reg [1:0] cur_state;
reg [1:0] nxt_state;
wire timeout_busy;
wire timer_start = cur_state == IDLE && nxt_state != IDLE;
reg [5:0] ch_cnt;
reg [7:0] word_cnt;
reg mosi_vld;
reg miso_vld_r;
wire miso_vld;
reg [3:0] load_nop_cnt;
assign tx_fifo_rd_en = mosi_vld && cur_state == WORK;
wire rx_fifo_full = (RX_FIFO_DEPTH - rx_fifo_data_count) < (RX_BLOCK_LEN * MAX_CHIP_IN_CH);
wire [31:0] miso_dat;
assign reg_state = {1'b0, cur_state};

always @ (posedge clk) begin
	if(rst)
		cur_state <= IDLE;
	else
		cur_state <= nxt_state;
end

always @ (*) begin
	nxt_state = cur_state;
	case(cur_state)
	IDLE:   if(~tx_fifo_empty && ~rx_fifo_full)
			nxt_state = WORK;
	WORK:	if(word_cnt == reg_word_num)
			nxt_state = NOP;
	NOP:	if(ch_cnt == reg_ch_num && &load_nop_cnt)
			nxt_state = DONE;
		else if(~tx_fifo_empty && ~rx_fifo_full && ch_cnt < reg_ch_num && &load_nop_cnt)
			nxt_state = WORK;
	DONE:	if(~timeout_busy && miso_vld_r)
			nxt_state = IDLE;
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
	else if(cur_state != DONE && nxt_state == DONE)
		mosi_vld <= 1'b1;//for load
	else
		mosi_vld <= 1'b0;
end

always @ (posedge clk) begin
	if(rst)
		miso_vld_r <= 1'b0;
	else if(miso_vld && cur_state == DONE)
		miso_vld_r <= 1'b1;
	else if(cur_state == IDLE)
		miso_vld_r <= 1'b0;
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

assign rx_fifo_wr_en = miso_vld && (word_cnt < RX_BLOCK_LEN) && (cur_state == WORK);
assign rx_fifo_din = miso_dat;

always @ (posedge clk) begin
	if(rst)
		load <= {`API_NUM{1'b1}};
	else if(cur_state == IDLE && nxt_state != IDLE)
		load <= {`API_NUM{1'b1}} ^ `API_NUM'b1;
	else if(cur_state == NOP && nxt_state == WORK)
		load <= {load[`API_NUM-2:0], 1'b1};
	else if(cur_state == NOP && nxt_state == DONE)
		load <= {`API_NUM{1'b1}};
end

always @ (posedge clk) begin
	if(rst)
		ch_cnt <= 6'b0;
	else if(nxt_state == IDLE)
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

endmodule
