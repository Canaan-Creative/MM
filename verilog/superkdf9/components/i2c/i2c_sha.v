module i2c_sha(
input             clk       ,
input             rst       ,

input             reg_sha_en,
input             reg_wstop ,

input             phy_push  ,
input      [31:0] phy_din   ,

output reg        fifo_push ,
output reg        pool_push ,
output reg [31:0] din        
);

parameter IDLE = 2'd0;
parameter BUF  = 2'd1;
parameter PUSH = 2'd2;

reg [1:0] cur_state;
reg [1:0] nxt_state;
reg [31:0] din_r;
reg is_cmd;

always @ (posedge clk) begin
	if(rst||reg_wstop)
		cur_state <= IDLE;
	else
		cur_state <= nxt_state;
end

always @ (*) begin
	nxt_state = cur_state;
	case(cur_state)
	IDLE :  if(reg_sha_en && phy_push && phy_din[15:8] == 8'd14 && is_cmd)
			nxt_state = BUF;
	BUF  :  if(phy_push)
			nxt_state = PUSH;
	PUSH :  if(reg_wstop)
			nxt_state = IDLE;
	endcase
end

always @ (posedge clk) begin
	if(rst || reg_wstop)
		is_cmd <= 1'b1;
	else if(phy_push)
		is_cmd <= 1'b0;
end

//-------------------------------------------------
// fifo zone
//-------------------------------------------------
always @ (posedge clk) begin
	if(~reg_sha_en || (cur_state == IDLE && nxt_state == IDLE)) begin
		fifo_push <= phy_push;
		pool_push <= 1'b0;
		din       <= phy_din;
	end else if(cur_state == BUF && phy_push) begin
		fifo_push <= 1'b0;
		pool_push <= 1'b0;
		din_r     <= phy_din;
	end else if(cur_state == PUSH && phy_push) begin
		fifo_push <= 1'b0;
		pool_push <= 1'b1;
		din_r     <= phy_din;
		din       <= {din_r[23:0], phy_din[31:24]};
	end else begin
		fifo_push <= 1'b0;
		pool_push <= 1'b0;
	end
end

endmodule
