`include "api_define.v"

module api_ctrl(
input                 clk               ,
input                 rst               ,

output [2:0]          reg_state         ,
input  [27:0]         reg_timeout       ,
input  [7:0]          reg_sck           ,
input  [5:0]          reg_ch_num        ,
input  [5:0]          reg_chip_num      ,

input                 tx_fifo_empty     ,
output reg            tx_fifo_rd_en     ,
input  [31:0]         tx_fifo_dout      ,

output                rx_fifo_wr_en     ,
output [31:0]         rx_fifo_din       ,
input  [8:0]          rx_fifo_data_count,

output reg [`API_NUM-1:0] load          ,
output                sck               ,
output                mosi              ,
input  [`API_NUM-1:0] miso
);
parameter RX_FIFO_DEPTH = 256;

wire timer_start, timeout_busy;
reg mosi_vld;
wire [31:0] mosi_dat;
wire miso_vld;
wire [31:0] miso_dat;
wire [31:0] din;
wire wr_en;
wire rd_en;
reg [4:0] word_cnt;
reg [5:0] chip_cnt;
reg [5:0] ch_cnt;
reg [8:0] dly1_cnt;
wire miso_vld_dly;

parameter WORK_LEN = 736/32;//words

parameter IDLE = 3'd0;
parameter FIFO = 3'd1;
parameter WORK = 3'd2;
parameter NOP  = 3'd3;
parameter LDUP = 3'd4;
parameter DONE = 3'd5;

reg [2:0] cur_state;
reg [2:0] nxt_state;

always @ (posedge clk) begin
	if(rst)
		cur_state <= IDLE;
	else
		cur_state <= nxt_state;
end

always @ (*) begin
	nxt_state = cur_state;
	case(cur_state)
	IDLE:   if(~tx_fifo_empty && |reg_ch_num && |reg_chip_num)
			nxt_state = FIFO;
	FIFO:   if(word_cnt == WORK_LEN)
			nxt_state = WORK;
	WORK:   if(word_cnt == WORK_LEN)
			nxt_state = NOP;
	NOP :   if(chip_cnt < reg_chip_num)
			nxt_state = WORK;
		else if(chip_cnt == reg_chip_num)
			nxt_state = LDUP;
	LDUP:   if(miso_vld_dly && ch_cnt != reg_ch_num - 1 && ~tx_fifo_empty)
			nxt_state = FIFO;
		else if(miso_vld_dly && ch_cnt == reg_ch_num - 1)
			nxt_state = DONE;
	DONE:   if(~timeout_busy)
			nxt_state = IDLE;
	default: nxt_state = IDLE;
	endcase
end

//----------------------------------------------
// timer
//----------------------------------------------
assign timer_start = cur_state == IDLE && nxt_state != IDLE;
assign reg_state = cur_state;

//----------------------------------------------
// word_cnt chip_cnt ch_cnt
//----------------------------------------------
always @ (posedge clk) begin
	if(rst)
		word_cnt <= 5'b0;
	else if(cur_state == FIFO && nxt_state == FIFO)
		word_cnt <= word_cnt + 5'b1;
	else if(cur_state == WORK && word_cnt != WORK_LEN && miso_vld)
		word_cnt <= word_cnt + 5'b1;
	else if((cur_state == FIFO && nxt_state == WORK)|| cur_state == NOP)
		word_cnt <= 5'b0;
end

always @ (posedge clk) begin
	if(rst)
		chip_cnt <= 6'b0;
	else if(cur_state == WORK && nxt_state == NOP)
		chip_cnt <= chip_cnt + 6'b1;
	else if(cur_state == LDUP)
		chip_cnt <= 6'b0;
end

always @ (posedge clk) begin
	if(rst)
		ch_cnt <= 6'b0;
	else if(cur_state == LDUP && nxt_state == FIFO)
		ch_cnt <= ch_cnt + 6'b1;
	else if(cur_state == IDLE)
		ch_cnt <= 6'b0;
end

//----------------------------------------------
// FSM: FIFO
//----------------------------------------------
always @ (posedge clk) begin
	if(cur_state == FIFO && nxt_state == FIFO)
		tx_fifo_rd_en <= 1'b1;
	else
		tx_fifo_rd_en <= 1'b0;
end

reg mosi_vld_f;
reg [31:0] mosi_dat_f;
always @ (posedge clk) begin
	if(cur_state == WORK && chip_cnt != reg_chip_num - 1 && mosi_vld)
		mosi_vld_f <= 1'b1;
	else
		mosi_vld_f <= 1'b0;
end

always @ (posedge clk) begin
	mosi_dat_f <= word_cnt == 'd2 ? mosi_dat + 1 : mosi_dat;
end

assign wr_en =  tx_fifo_rd_en | mosi_vld_f;
assign din   =  tx_fifo_rd_en ? tx_fifo_dout : mosi_dat_f;

//----------------------------------------------
// miso
//----------------------------------------------
reg rx_fifo_en;
always @ (posedge clk) begin
	if(rst)
		rx_fifo_en <= 1'b0;
	else if(cur_state != WORK && nxt_state == WORK && rx_fifo_data_count < RX_FIFO_DEPTH - 5)
		rx_fifo_en <= 1'b1;
	else if(word_cnt == 4)
		rx_fifo_en <= 1'b0;
end

assign rx_fifo_wr_en = miso_vld && word_cnt < 4 && rx_fifo_en && cur_state == WORK;
assign rx_fifo_din = miso_dat;

//----------------------------------------------
// mosi
//----------------------------------------------
reg [9:0] dly_cnt;
always @ (posedge clk) begin
	if(rst)
		dly_cnt <= 10'b0;
	else if(cur_state == NOP && nxt_state == LDUP)
		dly_cnt <= 10'b1;
	else if(dly_cnt != {reg_sck,2'b00} && |dly_cnt)
		dly_cnt <= dly_cnt + 10'b1;
	else
		dly_cnt <= 10'b0;
end

always @ (posedge clk) begin
	if(rst)
		dly1_cnt <= 9'b0;
	else if(cur_state == LDUP && miso_vld)
		dly1_cnt <= 9'b1;
	else if(dly1_cnt != {reg_sck, 1'b0} && |dly1_cnt)
		dly1_cnt <= 9'b1 + dly1_cnt;
	else
		dly1_cnt <= 9'b0;
end

assign miso_vld_dly = dly1_cnt == {reg_sck, 1'b0} && |dly1_cnt;

always @ (posedge clk) begin
	if(cur_state != WORK && nxt_state == WORK)
		mosi_vld <= 1'b1;
	else if(cur_state == WORK && (word_cnt != WORK_LEN - 1) && miso_vld)
		mosi_vld <= 1'b1;
	else if(dly_cnt == {reg_sck,2'b00} && |dly_cnt)
		mosi_vld <= 1'b1;//load up 32 scl posedge
	else
		mosi_vld <= 1'b0;
end

reg [`API_NUM-1:0] load_r;

always @ (posedge clk) begin
	if(rst)
		load <= {`API_NUM{1'b1}};
	else if(cur_state == IDLE && nxt_state == FIFO)
		load <= {`API_NUM{1'b1}} ^ `API_NUM'b1;
	else if(cur_state == LDUP && dly_cnt == {reg_sck,1'b0})
		load <= {`API_NUM{1'b1}};
	else if(cur_state == LDUP && nxt_state == FIFO)
		load <= load_r;
	else if(cur_state == LDUP && nxt_state == DONE)
		load <= {`API_NUM{1'b1}};
	else if(cur_state == DONE)
		load <= {`API_NUM{1'b1}};		
end

always @ (posedge clk) begin
	if(cur_state == NOP && nxt_state == LDUP)
		load_r <= {load[`API_NUM-2:0], 1'b1};
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
/*input  [31:0]  */ .mosi_dat    (mosi_dat    ),

/*output         */ .miso_vld    (miso_vld    ),
/*output [31:0]  */ .miso_dat    (miso_dat    ),

/*output         */ .sck         (sck         ),
/*output         */ .mosi        (mosi        ),
/*input          */ .miso        (miso_w      )
);

fifo32 fifo32(
/*input          */ .clk         (clk         ),
/*input          */ .srst        (rst         ),
/*input  [31 : 0]*/ .din         (din         ),
/*input          */ .wr_en       (wr_en       ),
/*input          */ .rd_en       (mosi_vld    ),
/*output [31 : 0]*/ .dout        (mosi_dat    ),
/*output         */ .full        (            ),
/*output         */ .empty       (            ),
/*output [4 : 0] */ .data_count  (            ) 
);

endmodule
