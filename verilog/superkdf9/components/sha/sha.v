
module sha(
    // system clock and reset
    input CLK_I,
    input RST_I,
    
    // wishbone interface signals
    input SHA_CYC_I,//NC
    input SHA_STB_I,
    input SHA_WE_I,
    input SHA_LOCK_I,//NC
    input [2:0] SHA_CTI_I,//NC
    input [1:0] SHA_BTE_I,//NC
    input [3:0] SHA_ADR_I,
    input [31:0]  SHA_DAT_I,
    input [3:0]   SHA_SEL_I,
    output reg    SHA_ACK_O,
    output        SHA_ERR_O,//const 0
    output        SHA_RTY_O,//const 0
    output [31:0] SHA_DAT_O 
);
assign SHA_ERR_O = 1'b0 ;
assign SHA_RTY_O = 1'b0 ;
//-----------------------------------------------------
// WB bus ACK
//-----------------------------------------------------
always @ ( posedge CLK_I or posedge RST_I ) begin
	if( RST_I )
		SHA_ACK_O <= 1'b0 ;
	else if( SHA_STB_I && (~SHA_ACK_O) )
		SHA_ACK_O <= 1'b1 ;
	else 
		SHA_ACK_O <= 1'b0 ;
end

wire sha_cmd_wr_en = SHA_STB_I & SHA_WE_I  & ( SHA_ADR_I == 4'h0) & ~SHA_ACK_O ;
wire sha_din_wr_en = SHA_STB_I & SHA_WE_I  & ( SHA_ADR_I == 4'h4) & ~SHA_ACK_O ;

wire sha_cmd_rd_en = SHA_STB_I & ~SHA_WE_I & ( SHA_ADR_I == 4'h0) & ~SHA_ACK_O ;
wire sha_hash_rd_en = SHA_STB_I & ~SHA_WE_I  & ( SHA_ADR_I == 4'h8) & ~SHA_ACK_O ;

//-----------------------------------------------------
// REG SHA_CMD
//-----------------------------------------------------
reg reg_init ;
reg reg_done ;
wire done ;
always @ ( posedge CLK_I or posedge RST_I ) begin
	if( RST_I )
		reg_init <= 1'b0 ;
	else if( sha_cmd_wr_en )
		reg_init <= SHA_DAT_I[0] ;
	else
		reg_init <= 1'b0 ;
end

always @ ( posedge CLK_I or posedge RST_I ) begin
	if( RST_I )
		reg_done <= 1'b0 ;
	else if( done )
		reg_done <= 1'b1 ;
	else if( sha_din_wr_en )
		reg_done <= 1'b0 ;
end


//-----------------------------------------------------
// REG SHA_DIN
//-----------------------------------------------------
reg [31:0] reg_din ;
reg vld ;
always @ ( posedge CLK_I or posedge RST_I ) begin
	if( RST_I ) begin
		reg_din <= 32'b0 ;
		vld <= 1'b0 ;
	end else if( sha_din_wr_en ) begin
		reg_din <= SHA_DAT_I[31:0] ;
		vld <= 1'b1 ;
	end else
		vld <= 1'b0 ;
end


//-----------------------------------------------------
// REG SHA_HASH
//-----------------------------------------------------
reg [2:0] hash_cnt ;
reg [31:0] reg_hash ;
wire [255:0] hash ;
always @ ( posedge CLK_I or posedge RST_I ) begin
	if( RST_I )
		reg_hash <= 32'b0 ;
	else if( sha_hash_rd_en ) begin
		case( hash_cnt )
		3'd0: reg_hash <= hash[32*8-1:32*7] ;
		3'd1: reg_hash <= hash[32*7-1:32*6] ;
		3'd2: reg_hash <= hash[32*6-1:32*5] ;
		3'd3: reg_hash <= hash[32*5-1:32*4] ;
		3'd4: reg_hash <= hash[32*4-1:32*3] ;
		3'd5: reg_hash <= hash[32*3-1:32*2] ;
		3'd6: reg_hash <= hash[32*2-1:32*1] ;
		3'd7: reg_hash <= hash[32*1-1:32*0] ;
		endcase
	end
end

always @ ( posedge CLK_I or posedge RST_I ) begin
	if( RST_I | reg_init )
		hash_cnt <= 3'b0 ;
	else if( sha_hash_rd_en )
		hash_cnt <= hash_cnt + 3'b1 ;
end

//-----------------------------------------------------
// Bus output
//-----------------------------------------------------
reg sha_cmd_rd_en_f ;
reg sha_hash_rd_en_f ;
always @ ( posedge CLK_I or posedge RST_I ) begin
	if( RST_I ) begin
		sha_cmd_rd_en_f <= 1'b0 ;
		sha_hash_rd_en_f <= 1'b0 ;
	end else begin
		sha_cmd_rd_en_f <= sha_cmd_rd_en ;
		sha_hash_rd_en_f <= sha_hash_rd_en ;
	end
end

assign SHA_DAT_O = sha_cmd_rd_en_f ? {30'h0,reg_done,1'b0} : reg_hash ; 

sha_core U_sha_core(
/*input         */ .clk     (CLK_I ) , 
/*input         */ .rst     (RST_I ),
        
/*input         */ .init    (reg_init),//sha core initial
/*input         */ .vld     (vld),//data input valid
/*input  [31:0] */ .din     (reg_din),

/*output        */ .done    (done),
/*output [255:0]*/ .hash    (hash)
) ;

endmodule 
