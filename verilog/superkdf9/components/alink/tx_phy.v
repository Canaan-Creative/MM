
`include "alink_define.v"
module tx_phy(
input             clk          ,
input             rst          ,

input             reg_flush    ,
input             reg_scan     ,

input             tx_phy_start ,
input  [31:0]     tx_phy_sel   ,
output            tx_phy_done  , 

input  [31:0]     tx_dout      ,//TxFIFO data input
output            tx_rd_en     ,//TxFIFO pop

output reg        task_id_vld  ,
output reg [31:0] rx_phy_sel   ,
output reg [31:0] task_id_h    ,
output reg [31:0] task_id_l    ,
output reg [31:0] reg_tout     ,

output     [31:0] TX_P         ,
output     [31:0] TX_N          
);
/*
                       __
tx_phy_start     _____|  |_____________________________________
                 _____ __ _____________________________________
tx_phy_sel       _____|0 |_________________dont care___________
                                                          __
tx_phy_done      ________________________________________|  |__
                                   __
task_id_vld      _________________|  |_________________________
                 _________________ __ _________________________
[rx_phy_sel      _________________|0_|_________________________
task_id_h
task_id_l]

*/

parameter IDLE = 2'd0 ;
parameter TASK = 2'd1 ;
parameter HASH = 2'd2 ;
parameter NONCE= 2'd3 ;

//----------------------------------------------
// Alink.clock.tick
//----------------------------------------------
reg [2:0] cur_state ;
reg [2:0] nxt_state ;
reg [4:0] word_cnt ;
reg [3:0] timing_cnt ;
reg       hash_pop ;
reg [31:0] tx_buf_flg ;
reg [31:0] tx_buf ;
reg [2:0] tx_rd_en_cnt ;
reg [31:0] reg_step    ;
always @ ( posedge clk ) begin
	if( rst || cur_state == IDLE )
		tx_rd_en_cnt <= 3'b0 ;
	else if( tx_rd_en && cur_state == TASK )
		tx_rd_en_cnt <= tx_rd_en_cnt + 3'b1 ;
end
 
always @ ( posedge clk ) begin
	if( rst )
		timing_cnt <= 4'b0 ;
	else if( timing_cnt == `TX_PHY_TIMING )
		timing_cnt <= 4'b0 ;
	else if( cur_state == HASH || cur_state == NONCE )
		timing_cnt <= timing_cnt + 4'b1 ;
	else
		timing_cnt <= 4'b0 ;
end

wire tick = ( timing_cnt == `TX_PHY_TIMING ) ;
reg nonce_over_flow ;
//----------------------------------------------
// FSM
//----------------------------------------------

always @ ( posedge clk ) begin
	if( rst )
		cur_state <= IDLE ;
	else
		cur_state <= nxt_state ;
end

always @ ( * ) begin
	nxt_state = cur_state ;
	case( cur_state )
	IDLE : if( tx_phy_start ) nxt_state = TASK ;
	TASK : if( tx_rd_en_cnt == `TX_TASKID_LEN-1 ) nxt_state = HASH ;
	HASH : if( word_cnt == `TX_DATA_LEN-1 && ~|tx_buf_flg ) nxt_state = NONCE ;
	NONCE: if( nonce_over_flow&&tick ) nxt_state = IDLE ;
	endcase
end

assign tx_phy_done = (cur_state == NONCE)&&(nxt_state == IDLE) ;
//----------------------------------------------
// TASK.ID
//----------------------------------------------
always @ ( posedge clk ) begin
	if( cur_state == IDLE && nxt_state == TASK ) begin
		rx_phy_sel <= tx_phy_sel ;
	end

	if( cur_state == TASK && tx_rd_en_cnt == 2'd0 ) task_id_h <= tx_dout ;
	if( cur_state == TASK && tx_rd_en_cnt == 2'd1 ) task_id_l <= tx_dout ;
	if( cur_state == TASK && tx_rd_en_cnt == 2'd2 ) reg_step  <= tx_dout ;
	if( cur_state == TASK && tx_rd_en_cnt == 2'd3 ) reg_tout  <= tx_dout ;

	if( rst )
		task_id_vld <= 1'b0 ;
	else if( cur_state == TASK && nxt_state == HASH )
		task_id_vld <= 1'b1 ;
	else 
		task_id_vld <= 1'b0 ;
end

wire [31:0] scan_nonce = task_id_l ;
wire [7:0]  scan_no   = task_id_h[7:0] ;
reg  [7:0]  scan_cnt ;

//----------------------------------------------
// Shifter
//----------------------------------------------
always @ ( posedge clk ) begin
	if( rst || cur_state == IDLE )
		word_cnt <= 5'b0 ;
	else if( cur_state == HASH && ~|tx_buf_flg )
		word_cnt <= word_cnt + 5'b1 ;
end

assign tx_rd_en = ( cur_state == TASK ) || ( hash_pop ) ;

reg TX_Px ;
reg TX_Nx ;
always @ ( posedge clk or posedge rst ) begin
	if( rst || (cur_state == NONCE && nxt_state == IDLE) || reg_flush ) begin
		TX_Px <= 1'b1 ;
		TX_Nx <= 1'b1 ;
	end else if( cur_state == IDLE && nxt_state == TASK ) begin //START
		TX_Px <= 1'b0 ;
		TX_Nx <= 1'b0 ;
	end else if( cur_state == HASH || cur_state == NONCE ) begin
		if( ~TX_Px && ~TX_Nx && tick ) begin
			TX_Px <= tx_buf[0]?1'b1:1'b0 ;
			TX_Nx <= (~tx_buf[0])?1'b1:1'b0 ;
		end else if( tick ) begin
			TX_Px <= 1'b0 ;
			TX_Nx <= 1'b0 ;
		end
	end
end

assign {TX_P[0 ],TX_N[0 ]} = rx_phy_sel[0 ] ? {TX_Px,TX_Nx} : 2'b11 ;
assign {TX_P[1 ],TX_N[1 ]} = rx_phy_sel[1 ] ? {TX_Px,TX_Nx} : 2'b11 ;
assign {TX_P[2 ],TX_N[2 ]} = rx_phy_sel[2 ] ? {TX_Px,TX_Nx} : 2'b11 ;
assign {TX_P[3 ],TX_N[3 ]} = rx_phy_sel[3 ] ? {TX_Px,TX_Nx} : 2'b11 ;
assign {TX_P[4 ],TX_N[4 ]} = rx_phy_sel[4 ] ? {TX_Px,TX_Nx} : 2'b11 ;
assign {TX_P[5 ],TX_N[5 ]} = rx_phy_sel[5 ] ? {TX_Px,TX_Nx} : 2'b11 ;
assign {TX_P[6 ],TX_N[6 ]} = rx_phy_sel[6 ] ? {TX_Px,TX_Nx} : 2'b11 ;
assign {TX_P[7 ],TX_N[7 ]} = rx_phy_sel[7 ] ? {TX_Px,TX_Nx} : 2'b11 ;
assign {TX_P[8 ],TX_N[8 ]} = rx_phy_sel[8 ] ? {TX_Px,TX_Nx} : 2'b11 ;
assign {TX_P[9 ],TX_N[9 ]} = rx_phy_sel[9 ] ? {TX_Px,TX_Nx} : 2'b11 ;
`ifdef PHY_10
assign {TX_P[10],TX_N[10]} = rx_phy_sel[10] ? {TX_Px,TX_Nx} : 2'b11 ;
assign {TX_P[11],TX_N[11]} = rx_phy_sel[11] ? {TX_Px,TX_Nx} : 2'b11 ;
assign {TX_P[12],TX_N[12]} = rx_phy_sel[12] ? {TX_Px,TX_Nx} : 2'b11 ;
assign {TX_P[13],TX_N[13]} = rx_phy_sel[13] ? {TX_Px,TX_Nx} : 2'b11 ;
assign {TX_P[14],TX_N[14]} = rx_phy_sel[14] ? {TX_Px,TX_Nx} : 2'b11 ;
assign {TX_P[15],TX_N[15]} = rx_phy_sel[15] ? {TX_Px,TX_Nx} : 2'b11 ;
assign {TX_P[16],TX_N[16]} = rx_phy_sel[16] ? {TX_Px,TX_Nx} : 2'b11 ;
assign {TX_P[17],TX_N[17]} = rx_phy_sel[17] ? {TX_Px,TX_Nx} : 2'b11 ;
assign {TX_P[18],TX_N[18]} = rx_phy_sel[18] ? {TX_Px,TX_Nx} : 2'b11 ;
assign {TX_P[19],TX_N[19]} = rx_phy_sel[19] ? {TX_Px,TX_Nx} : 2'b11 ;
assign {TX_P[20],TX_N[20]} = rx_phy_sel[20] ? {TX_Px,TX_Nx} : 2'b11 ;
assign {TX_P[21],TX_N[21]} = rx_phy_sel[21] ? {TX_Px,TX_Nx} : 2'b11 ;
assign {TX_P[22],TX_N[22]} = rx_phy_sel[22] ? {TX_Px,TX_Nx} : 2'b11 ;
assign {TX_P[23],TX_N[23]} = rx_phy_sel[23] ? {TX_Px,TX_Nx} : 2'b11 ;
assign {TX_P[24],TX_N[24]} = rx_phy_sel[24] ? {TX_Px,TX_Nx} : 2'b11 ;
assign {TX_P[25],TX_N[25]} = rx_phy_sel[25] ? {TX_Px,TX_Nx} : 2'b11 ;
assign {TX_P[26],TX_N[26]} = rx_phy_sel[26] ? {TX_Px,TX_Nx} : 2'b11 ;
assign {TX_P[27],TX_N[27]} = rx_phy_sel[27] ? {TX_Px,TX_Nx} : 2'b11 ;
assign {TX_P[28],TX_N[28]} = rx_phy_sel[28] ? {TX_Px,TX_Nx} : 2'b11 ;
assign {TX_P[29],TX_N[29]} = rx_phy_sel[29] ? {TX_Px,TX_Nx} : 2'b11 ;
assign {TX_P[30],TX_N[30]} = rx_phy_sel[30] ? {TX_Px,TX_Nx} : 2'b11 ;
assign {TX_P[31],TX_N[31]} = rx_phy_sel[31] ? {TX_Px,TX_Nx} : 2'b11 ;
`else
assign {TX_P[10],TX_N[10]} = 2'b11 ;
assign {TX_P[11],TX_N[11]} = 2'b11 ;
assign {TX_P[12],TX_N[12]} = 2'b11 ;
assign {TX_P[13],TX_N[13]} = 2'b11 ;
assign {TX_P[14],TX_N[14]} = 2'b11 ;
assign {TX_P[15],TX_N[15]} = 2'b11 ;
assign {TX_P[16],TX_N[16]} = 2'b11 ;
assign {TX_P[17],TX_N[17]} = 2'b11 ;
assign {TX_P[18],TX_N[18]} = 2'b11 ;
assign {TX_P[19],TX_N[19]} = 2'b11 ;
assign {TX_P[20],TX_N[20]} = 2'b11 ;
assign {TX_P[21],TX_N[21]} = 2'b11 ;
assign {TX_P[22],TX_N[22]} = 2'b11 ;
assign {TX_P[23],TX_N[23]} = 2'b11 ;
assign {TX_P[24],TX_N[24]} = 2'b11 ;
assign {TX_P[25],TX_N[25]} = 2'b11 ;
assign {TX_P[26],TX_N[26]} = 2'b11 ;
assign {TX_P[27],TX_N[27]} = 2'b11 ;
assign {TX_P[28],TX_N[28]} = 2'b11 ;
assign {TX_P[29],TX_N[29]} = 2'b11 ;
assign {TX_P[30],TX_N[30]} = 2'b11 ;
assign {TX_P[31],TX_N[31]} = 2'b11 ;
`endif
reg [32:0] nonce_buf ;                        
always @ ( posedge clk or posedge rst ) begin
	if( rst ) begin
		hash_pop <= 1'b0 ;
	end else if( cur_state == IDLE && nxt_state == TASK ) begin
		hash_pop <= 1'b0 ;
	end else if( ~TX_Px && ~TX_Nx && tick ) begin
		hash_pop <= 1'b0 ;
	end else if( cur_state == TASK && nxt_state == HASH ) begin
        	hash_pop <= 1'b1 ;
	end else if( cur_state == HASH && nxt_state != NONCE && ~|tx_buf_flg ) begin
        	hash_pop <= 1'b1 ;
	end else begin
		hash_pop <= 1'b0 ;
	end
end

always @ ( posedge clk or posedge rst ) begin
	if( rst ) begin
		tx_buf <= 32'b0 ;
		nonce_over_flow <= 1'b0 ;
		nonce_buf <= 33'b0 ;
		scan_cnt <= 8'b0 ;
	end else if( cur_state == IDLE && nxt_state == TASK ) begin
		nonce_over_flow <= 1'b0 ;
		nonce_buf <= 33'b0 ;
		scan_cnt <= 8'b0 ;
	end else if( ~TX_Px && ~TX_Nx && tick ) begin
		tx_buf <= {1'b0,tx_buf[31:1]} ;
	end else if( hash_pop ) begin
		tx_buf <= tx_dout ;
	end else if( cur_state == HASH && nxt_state == NONCE ) begin
		tx_buf <= (reg_scan && (scan_no == scan_cnt)) ? scan_nonce : {32{reg_scan}} | 32'b0 ;
		nonce_buf <= nonce_buf + {1'b0,reg_step} ;
		scan_cnt <= reg_scan + scan_cnt ;
	end else if( cur_state == NONCE && ~|tx_buf_flg ) begin
		tx_buf <= (reg_scan && (scan_no == scan_cnt)) ? scan_nonce : {32{reg_scan}} | nonce_buf[31:0] ;
		nonce_buf <= nonce_buf + {1'b0,reg_step} ;
		nonce_over_flow <= ((nonce_buf)>33'hffff_ffff) ? 1'b1:1'b0 ;
		scan_cnt <= reg_scan + scan_cnt ;
	end
end

always @ ( posedge clk or posedge rst ) begin
	if( rst || cur_state == IDLE ) begin
		tx_buf_flg <= 32'hffffffff ;
	end else if( ~TX_Px && ~TX_Nx && tick ) begin
		tx_buf_flg <= {1'b0,tx_buf_flg[31:1]} ;
	end else if( (cur_state == HASH || cur_state == NONCE) && ~|tx_buf_flg ) begin
		tx_buf_flg <= 32'hffffffff ;
	end
end

endmodule
