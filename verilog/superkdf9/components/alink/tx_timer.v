
`include "alink_define.v"
module tx_timer(
input             clk          ,
input             rst          ,
input             reg_flush    ,
input  [31:0]     reg_tout     ,
input             timer_start  ,
output            timer_done   ,
output [31:0]     timer_cnt        
);

reg [31:0] timer ;

assign timer_cnt = timer ;

always @ ( posedge clk ) begin
	if( rst || (timer == reg_tout) || reg_flush )
		timer <= 32'b0 ;
	else if( timer_start )
		timer <= 32'b1 ;
	else if( |timer && (timer < reg_tout))
		timer <= 32'b1 + timer ;
end

assign timer_done = (timer == reg_tout[31:0]) && |reg_tout[31:0] ;

endmodule
