module api_timer(
input         clk         ,
input         rst         ,

input         reg_rst     ,
input  [27:0] reg_timeout ,
input         start       ,
output        timeout_busy
);

reg [27:0] reg_timeout_cnt;
always @ (posedge clk or posedge rst) begin
	if(rst)
		reg_timeout_cnt <= 28'b0;
	else if(reg_rst)
		reg_timeout_cnt <= 28'b0;
	else if(start)
		reg_timeout_cnt <= 28'b1;
	else if(|reg_timeout_cnt && reg_timeout_cnt < reg_timeout)
		reg_timeout_cnt <= reg_timeout_cnt + 28'b1;
	else
		reg_timeout_cnt <= 28'b0;
end

assign timeout_busy = |reg_timeout_cnt;

endmodule
