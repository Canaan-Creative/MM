module brg_shift(
input        clk ,
input        rst ,
input        vld ,
input  [7:0] din ,
output       done,

output       brg_sck ,
output       brg_mosi
);
reg [4:0] shcp_cnt ;
always @ ( posedge clk ) begin
        if( rst )
                shcp_cnt <= 0 ;
        else if( vld )
                shcp_cnt <= 1 ;
        else if( |shcp_cnt )
                shcp_cnt <= shcp_cnt + 1 ;
end

assign brg_sck = shcp_cnt[1] ;

reg [7:0] data ;
always @ ( posedge clk ) begin
        if( vld )
                data <= din ;
        else if( &shcp_cnt[1:0] )
                data <= data >> 1 ;
end

assign brg_mosi = vld ? din[0] : data[0] ;

assign done = shcp_cnt == 31 ;

endmodule

