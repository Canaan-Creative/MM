module sha_bulk(
input        clk           ,
input        rst           ,

output reg   pool_pop      ,
input        pool_empty    ,

input        reg_bulk_start,
output       reg_bulk_done ,
output       bulk_init_def ,
output reg   bulk_dbl      ,

output       sha_vld       ,
output [31:0]sha_din       ,
input        sha_done      ,
input [255:0]sha_hash       
);

parameter IDLE = 2'd0;
parameter PDAT = 2'd1;//pool data pop
parameter DUL  = 2'd2;
parameter DDAT = 2'd3;

reg [1:0] cur_state;
reg [1:0] nxt_state;
reg [255:0] sha_hash_r;
reg [3:0] word_cnt;
reg sha_done_f;

always @ (posedge clk) begin
	if(rst)
		cur_state <= IDLE;
	else
		cur_state <= nxt_state;
end

always @ (*) begin
	nxt_state = cur_state;
	case(cur_state)
	IDLE:   if(reg_bulk_start)
			nxt_state = PDAT;
	PDAT:   if(sha_done_f)
			nxt_state = DUL;
	DUL :   if(sha_done_f && pool_empty)
			nxt_state = IDLE;
		else if(sha_done_f && ~pool_empty)
			nxt_state = DDAT;
	DDAT:   if(word_cnt == 4'd7)
			nxt_state = PDAT;
	endcase
end

always @ (posedge clk) begin
	sha_done_f <= sha_done;
end

always @ (posedge clk) begin
	if(sha_done)
		sha_hash_r <= sha_hash;
	else if(cur_state == DDAT)
		sha_hash_r <= sha_hash << 32;
end

assign sha_vld = cur_state == DDAT;
assign sha_din = sha_hash_r[255:255-31];

always @ (posedge clk) begin
	if(word_cnt >= 4'd7 && word_cnt < 4'd15)
		pool_pop <= 1'b1;
	else
		pool_pop <= 1'b0;
end

always @ (posedge clk) begin
	if(cur_state == IDLE)
		word_cnt <= 4'd7;
	else if(cur_state == PDAT && |word_cnt)
		word_cnt <= word_cnt + 4'd1;
	else if(cur_state == DDAT)
		word_cnt <= word_cnt + 4'd1;
end

always @ (posedge clk) begin
	if(cur_state == PDAT && nxt_state == DUL)
		bulk_dbl <= 1'b1;
	else
		bulk_dbl <= 1'b0;
end

assign reg_bulk_done = cur_state == DUL && nxt_state == IDLE;
assign bulk_init_def = (cur_state == PDAT && nxt_state == DUL) || (cur_state == DUL && nxt_state == DDAT);

endmodule
