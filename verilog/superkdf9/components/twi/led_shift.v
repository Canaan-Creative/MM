module led_shift(
input        clk      ,
input        rst      ,
input        vld      ,
input  [7:0] din      ,
output       done     ,

output       sft_shcp ,
output       sft_ds    
);

//--------------------------------------------------
// shcp counter
//--------------------------------------------------
reg [3:0] shcp_cnt ;
always @ ( posedge clk ) begin
	if( rst )
		shcp_cnt <= 0 ;
	else if( vld )
		shcp_cnt <= 1 ;
	else if( |shcp_cnt )
		shcp_cnt <= shcp_cnt + 1 ;
end

assign sft_shcp = shcp_cnt[0] ;

reg [7:0] data ;
always @ ( posedge clk ) begin
	if( vld )
		data <= din ;
	else if( shcp_cnt[0] )
		data <= data >> 1 ;
end

assign sft_ds = vld ? din[0] : data[0] ;

//--------------------------------------------------
// done
//--------------------------------------------------
assign done = shcp_cnt == 15 ;

endmodule
