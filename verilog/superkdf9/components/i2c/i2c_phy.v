module i2c_phy(
input         clk      ,
input         rst      ,
input         scl_pin  ,
inout         sda_pin  ,

input  [6:0]  reg_addr ,
output reg    reg_wstop,//a write success
output reg    reg_rstop,//a read success
output reg    reg_rerr ,//a read error

input         full    ,
output reg    push    ,
output [31:0] dout    ,

input         empty   ,
output reg    pop     ,
input  [31:0] din     ,

output        led_iic_wr,
output        led_iic_rd 
);

reg [3:0] scl_f;
reg [3:0] sda_f;
reg scl, scl_r;
reg sda, sda_r, sda_o;
reg [2:0] bit_cnt;
reg [1:0] byte_cnt;
wire i2c_start = scl_r &&  sda_r && scl && ~sda;
wire i2c_stop  = scl_r && ~sda_r && scl &&  sda;
wire i2c_pos   = ~scl_r && scl;
wire i2c_neg   = scl_r && ~scl;
reg [31:0] sda_buf;
reg rw_flg;
wire get_8bit = &bit_cnt && i2c_neg;
reg acki_f;
reg i2c_start_r;
wire addr_ack;
wire addr_noack;
reg addr_noack_r;

assign sda_pin = sda_o ? 1'bz : 1'b0;

always @ (posedge clk) begin
	scl_f <= {scl_f[2:0], scl_pin};
	sda_f <= {sda_f[2:0], sda_pin};
end

always @ (posedge clk) begin
	if(scl_f == 4'b1111)
		scl <= 1'b1;
	else if(scl_f == 4'b0000)
		scl <= 1'b0;
	scl_r <= scl;
end

always @ (posedge clk) begin
	if(sda_f == 4'b1111)
                sda <= 1'b1;
        else if(sda_f == 4'b0000)
                sda <= 1'b0;
	sda_r <= sda;
end

parameter IDLE  = 3'd0;
parameter ADDR  = 3'd1;
parameter DWR   = 3'd2;
parameter DRD   = 3'd3;
parameter ACKO  = 3'd4;
parameter AACKO = 3'd5;
parameter ACKI  = 3'd6;

reg [2:0] cur_state;
reg [2:0] nxt_state;

always @ (posedge clk) begin
	if(rst || i2c_stop || i2c_start)
		cur_state <= IDLE;
	else
		cur_state <= nxt_state;
end

always @ (*) begin
	nxt_state = cur_state;
	case(cur_state)
	IDLE :  if(i2c_start_r && i2c_neg)
			nxt_state = ADDR;
	ADDR :  if(get_8bit)
			nxt_state = AACKO;
	DWR  :  if(get_8bit)
			nxt_state = ACKO;
		else if(i2c_start || addr_noack_r || i2c_stop)
			nxt_state = IDLE;
	DRD  :  if(i2c_pos && sda != sda_o)
			nxt_state = IDLE;
		else if(get_8bit)
			nxt_state = ACKI;
		else if(i2c_stop || addr_noack_r || i2c_start)
			nxt_state = IDLE;
	ACKO :  if(i2c_neg)
			nxt_state = DWR;
	AACKO:  if(i2c_neg && rw_flg)
			nxt_state = DRD;
		else if(i2c_neg)
			nxt_state = DWR;
	ACKI :  if(i2c_neg && ~acki_f)
			nxt_state = DRD;
		else if(i2c_neg)
			nxt_state = IDLE;
	default: nxt_state = IDLE;
	endcase
end

assign led_iic_wr = cur_state == AACKO && nxt_state == DWR;
assign led_iic_rd = cur_state == AACKO && nxt_state == DRD;

//-----------------------------------------------------
// I2C Bit Byte Counter 0~7
//-----------------------------------------------------
always @ (posedge clk) begin
	if(rst)
		bit_cnt <= 3'b0;
	else if(cur_state == IDLE)
		bit_cnt <= 3'b0;
	else if((cur_state == ADDR || cur_state == DWR || cur_state == DRD) && i2c_neg)
		bit_cnt <= 3'b1 + bit_cnt;
end

always @ (posedge clk) begin
	if(rst || cur_state == IDLE)
		byte_cnt <= 2'b0;
	else if(cur_state == DWR && nxt_state == ACKO)
		byte_cnt <= byte_cnt + 2'b1;
	else if(cur_state == DRD && nxt_state == ACKI)
		byte_cnt <= byte_cnt + 2'b1;
end

always @ (posedge clk) begin
	if(rst)
		i2c_start_r <= 1'b0;
	else if(i2c_start)
		i2c_start_r <= 1'b1;
	else if(i2c_neg)
		i2c_start_r <= 1'b0;
end

//-----------------------------------------------------
// SDA
//-----------------------------------------------------
always @ (posedge clk) begin
	if(rst)
		acki_f <= 1'b1;
	else if((cur_state == AACKO || cur_state == ACKI) && i2c_pos)
		acki_f <= sda;
end

assign addr_ack =       cur_state == ADDR && nxt_state == AACKO &&
			((~sda_buf[0] && ~full) || (sda_buf[0] && ~empty)) &&
			(sda_buf[7:1] == reg_addr || ~|sda_buf[7:1]);
assign addr_noack =     cur_state == ADDR && nxt_state == AACKO && ~addr_ack;

always @ (posedge clk) begin
	if(rst || cur_state == IDLE)
		sda_o <= 1'b1;
	else if(addr_ack)
		sda_o <= 1'b0;
	else if(cur_state == ACKO && i2c_neg)
		sda_o <= 1'b1;
	else if(cur_state == AACKO && i2c_neg && ~addr_noack_r)
		sda_o <= rw_flg ? sda_buf[31] : 1'b1;
	else if(cur_state == DRD && nxt_state == DRD && i2c_neg)
		sda_o <= sda_buf[31];
	else if(cur_state == DRD && nxt_state != DRD)
		sda_o <= 1'b1;
	else if(cur_state != DRD && nxt_state == DRD)
		sda_o <= acki_f ? 1'b1 : sda_buf[31];
	else if(cur_state == DWR && nxt_state == ACKO)
		sda_o <= 1'b0;
	else if(cur_state == ACKO && i2c_neg)
		sda_o <= 1'b1;
end

always @ (posedge clk) begin
	if(rst || cur_state == IDLE)
		addr_noack_r <= 1'b0;
	else if(addr_noack)
		addr_noack_r <= 1'b1;
end

//-----------------------------------------------------
// Pop Buffer
//-----------------------------------------------------
always @ (posedge clk) begin
	if(rst)
		pop <= 1'b0;
	else if(cur_state == ACKI && i2c_pos && ~sda_o && ~|byte_cnt)
		pop <= 1'b1;
	else if(cur_state == AACKO && i2c_pos && ~sda_o && sda_buf[0])
		pop <= 1'b1;
	else
		pop <= 1'b0;
end

//-----------------------------------------------------
// Push Buffer
//-----------------------------------------------------
always @ (posedge clk) begin
	if(rst)
		push <= 1'b0;
	else if(cur_state == DWR && nxt_state == ACKO && &byte_cnt)
		push <= 1'b1;
	else
		push <= 1'b0;
end

//-----------------------------------------------------
// SDA Buffer
//-----------------------------------------------------
always @ (posedge clk) begin
	if(rst)
		sda_buf <= 'b0;
	else if(pop)
		sda_buf <= din;
	else if(cur_state == DRD && i2c_pos)
		sda_buf <= {sda_buf[30:0], 1'b0};
	else if((cur_state == DWR || cur_state == ADDR) && i2c_pos)
		sda_buf <= {sda_buf[30:0], sda};
end

always @ (posedge clk) begin
	if(rst)
		rw_flg <= 1'b0;
	else if(cur_state == ADDR && i2c_pos)
		rw_flg <= sda;
end

assign dout = sda_buf;

always @ (posedge clk) begin
	if(rst)
		reg_wstop <= 1'b0;
	else if(cur_state == DWR && (i2c_stop || i2c_start))
		reg_wstop <= 1'b1;
	else
		reg_wstop <= 1'b0;
end

always @ (posedge clk) begin
	if(rst)
		reg_rstop <= 1'b0;
	else if(cur_state == ACKI && nxt_state == IDLE)
		reg_rstop <= 1'b1;
	else
		reg_rstop <= 1'b0;
end

always @ (posedge clk) begin
	if(rst)
		reg_rerr <= 1'b0;
	else if(cur_state == DRD && i2c_pos && sda != sda_o)
		reg_rerr <= 1'b1;
	else
		reg_rerr <= 1'b0;
end

endmodule
