module reboot(
input         clk          ,
input         reg_rbt_start,//only one pose
input         i2c_wr_stop  ,
input         i2c_rd_stop  ,

input         rx_vld       ,
input  [31:0] rx_dat       ,
input         tx_vld       ,
output reg [31:0] tx_dat       ,

output reg    rbt_enable   ,

output reg    ram_sel      ,//0: iram, 1: dram
output reg        ram_wr       ,
output reg [15:0] ram_addr     ,
output reg [31:0] ram_dat_wr   ,
input  [31:0] ram_dat_rd    
);
parameter IDLE = 3'd0;
parameter RBT  = 3'd1;//reboot enable
parameter WRAM = 3'd2;//write ram
parameter RRAM = 3'd3;//read ram
parameter RCRC = 3'd4;//read crc32
parameter RUN  = 3'd5;//run, release rbt_rst_cpu & rbt_enable

parameter CMD_WRAM = 8'h00;
parameter CMD_RRAM = 8'h01;
parameter CMD_RCRC = 8'h02;
parameter CMD_RUN  = 8'h03;

reg [2:0] cur_state;
reg [2:0] nxt_state;
reg [15:0] pc;
reg [31:0] ram_dat_rd_r;

always @ (posedge clk) begin
	cur_state <= nxt_state;
end

always @ (*) begin
	nxt_state = cur_state;
	case(cur_state)
	IDLE:   if(reg_rbt_start)
			nxt_state = RBT;
	RBT :   if(rx_vld && rx_dat[7:0] == CMD_WRAM)
			nxt_state = WRAM;
		else if(rx_vld && rx_dat[7:0] == CMD_RRAM)
			nxt_state = RRAM;
		else if(rx_vld && rx_dat[7:0] == CMD_RCRC)
			nxt_state = RCRC;
		else if(rx_vld && rx_dat[7:0] == CMD_RUN)
			nxt_state = RUN;
	WRAM:   if(i2c_wr_stop)
			nxt_state = RBT;
	RRAM:   if(i2c_rd_stop)
			nxt_state = RBT;
	RCRC:   if(i2c_rd_stop)
			nxt_state = RBT;
	RUN :   if(i2c_wr_stop)
			nxt_state = IDLE;
	endcase
end

//pc
always @ (posedge clk) begin
	if(cur_state == RBT && (nxt_state == WRAM || nxt_state == RRAM))
		pc <= rx_dat[31:16];
	else if(rx_vld && cur_state == WRAM)
		pc <= pc + 16'b1;
	else if(tx_vld && cur_state == RRAM)
		pc <= pc + 16'b1;
end

//write ram
always @ (posedge clk) begin
	ram_wr     <= cur_state == WRAM && rx_vld;
	ram_addr   <= pc;
	ram_dat_wr <= rx_dat;
	tx_dat     <= ram_dat_rd;
end

//read ram

always @ (posedge clk) begin
	if(cur_state == RBT && (nxt_state == WRAM || nxt_state == RRAM))
		ram_sel <= rx_dat[8];
end

//rbt_enable
always @ (posedge clk) begin
	if(cur_state != IDLE)
		rbt_enable <= 1'b1;
	else
		rbt_enable <= 1'b0;
end

initial rbt_enable = 0;
initial ram_sel    = 0;//0: iram, 1: dram
initial ram_wr     = 0;
initial ram_addr   = 0;
initial ram_dat_wr = 0;
initial cur_state   = 0;
initial nxt_state   = 0;
initial pc          = 0;
initial ram_dat_rd_r= 0;

endmodule
