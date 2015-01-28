module brg_shift(
input        clk ,
input        rst ,
input        reg_rst,
input        vld ,
input  [7:0] din ,
output       done,

output       brg_sck ,
output       brg_mosi
);
reg [4:0] shcp_cnt ;
always @ ( posedge clk or posedge rst) begin
        if( rst )
                shcp_cnt <= 5'b0 ;
	else if(reg_rst)
                shcp_cnt <= 5'b0 ;
        else if( vld )
                shcp_cnt <= 5'b1 ;
        else if( |shcp_cnt )
                shcp_cnt <= shcp_cnt + 5'b1 ;
end

assign brg_sck = shcp_cnt[1] ;

reg [7:0] data ;
always @ ( posedge clk or posedge rst) begin
	if(rst)
		data <= 8'b0;
        else if( vld )
                data <= din ;
        else if( &shcp_cnt[1:0] )
                data <= data >> 1 ;
end

assign brg_mosi = vld ? din[0] : data[0] ;

assign done = shcp_cnt == 5'd31 ;

endmodule

