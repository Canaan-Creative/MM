module api_phy(
input         clk      ,
input         rst      ,

input         reg_rst  ,
input  [7:0]  reg_sck  ,

input         mosi_vld ,
input  [31:0] mosi_dat ,

output reg       miso_vld ,
output reg [31:0] miso_dat ,

output        sck      ,
output        mosi     ,
input         miso
);

reg [5:0] bit_cnt;//0 to 63
reg [7:0] sck_cnt;
reg run;

always @ (posedge clk or posedge rst) begin
	if(rst)
		run <= 1'b0;
	else if(reg_rst)
		run <= 1'b0;
	else if(mosi_vld)
		run <= 1'b1;
	else if(miso_vld)
		run <= 1'b0;
end

always @ (posedge clk or posedge rst) begin
	if(rst)
		sck_cnt <= 8'b0;
	else if(reg_rst)
		sck_cnt <= 8'b0;
	else if(run && sck_cnt < reg_sck)
		sck_cnt <= sck_cnt + 8'b1;
	else
		sck_cnt <= 8'b0;
end

always @ (posedge clk or posedge rst) begin
	if(rst)
		bit_cnt <= 6'b0;
	else if(reg_rst)
		bit_cnt <= 6'b0;
	else if(run && sck_cnt == reg_sck)
		bit_cnt <= bit_cnt + 6'b1;
end

assign sck = bit_cnt[0];

reg [31:0] mosi_buf;
always @ (posedge clk or posedge rst) begin
	if(rst)
		mosi_buf <= 32'b0;
	else if(reg_rst)
		mosi_buf <= 32'b0;
	else if(mosi_vld)
		mosi_buf <= mosi_dat;
	else if(bit_cnt[0] && sck_cnt == reg_sck)
		mosi_buf <= {mosi_buf[30:0], 1'b0};
end

assign mosi = mosi_buf[31];

always @ (posedge clk or posedge rst) begin
	if(rst)
		miso_dat <= 32'b0;
	else if(reg_rst)
		miso_dat <= 32'b0;
	else if(~bit_cnt[0] && sck_cnt == reg_sck)
		miso_dat <= {miso_dat[30:0], miso};
end

always @ (posedge clk or posedge rst) begin
	if(rst)
		miso_vld <= 1'b0;
	else if(reg_rst)
		miso_vld <= 1'b0;
	else if(&bit_cnt && sck_cnt == reg_sck)
		miso_vld <= 1'b1;
	else
		miso_vld <= 1'b0;
end

endmodule
