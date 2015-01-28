`timescale 1ns / 100ps
module i2c(
input                 CLK_I     ,
input                 RST_I     ,

input                 I2C_CYC_I ,//NC
input                 I2C_STB_I ,
input                 I2C_WE_I  ,
input                 I2C_LOCK_I,//NC
input  [2:0]          I2C_CTI_I ,//NC
input  [1:0]          I2C_BTE_I ,//NC
input  [5:0]          I2C_ADR_I ,
input  [31:0]         I2C_DAT_I ,
input  [3:0]          I2C_SEL_I ,
output reg            I2C_ACK_O ,
output                I2C_ERR_O ,//const 0
output                I2C_RTY_O ,//const 0
output reg [31:0]     I2C_DAT_O ,

input                 scl_pin   ,
inout                 sda_pin   ,

output                int_i2c   ,

output                rbt_enable,
output                ram_sel   ,//0: iram, 1: dram
output                ram_wr    ,
output [15:0]         ram_addr  ,
output [31:0]         ram_dat_wr,
input  [31:0]         ram_dat_rd,

output        led_iic_wr,
output        led_iic_rd,

output        brg_en,
output        brg_cs,
output        brg_sck,
output        brg_mosi
);

assign I2C_RTY_O = 1'b0;
assign I2C_ERR_O = 1'b0;
parameter PKG_LEN = 10;

reg  [6:0] reg_addr ;
wire       reg_wstop;//a write success
wire       reg_rstop;//a read success
wire       reg_rerr ;//a read error
reg        reg_txrst;
reg        reg_rxrst;
reg        reg_rst;

wire i2c_ctrl_wr_en;
wire i2c_ctrl_rd_en;
wire i2c_tx_wr_en  ;
wire i2c_rx_rd_en  ;

wire          tx_rst       = RST_I | reg_txrst;
reg  [31 : 0] tx_din       ;
reg           tx_wr_en     ;
wire          tx_rd_en     ;
wire [31 : 0] tx_dout      ;
wire [8 : 0]  tx_data_count;

wire          rx_rst       = RST_I | reg_rxrst;
wire [31 : 0] rx_din       ;
wire          rx_wr_en     ;
wire          rx_rd_en     = i2c_rx_rd_en;
wire [31 : 0] rx_dout      ;
wire [8 : 0]  rx_data_count;

wire [6:0] addr_r;
wire [7:0] byte_buf;
wire       byte_done;

wire          full         = rbt_enable ? 1'b0 : (rx_data_count + PKG_LEN >= 256);
wire          empty        = rbt_enable ? 1'b0 : (tx_data_count < PKG_LEN);

wire [31:0] tx_dat;
parameter I2C_CTRL = 6'h00;
parameter I2C_ADDR = 6'h04;
parameter I2C_TX   = 6'h08;
parameter I2C_RX   = 6'h0c;
parameter I2C_DNA  = 6'h10;
parameter I2C_RBT  = 6'h14;//reboot

always @ (posedge CLK_I or posedge RST_I) begin
        if(RST_I)
                I2C_ACK_O <= 1'b0;
        else if(I2C_STB_I && (~I2C_ACK_O))
                I2C_ACK_O <= 1'b1;
        else 
                I2C_ACK_O <= 1'b0;
end

assign i2c_ctrl_wr_en = I2C_STB_I & I2C_WE_I  & (I2C_ADR_I == I2C_CTRL) & ~I2C_ACK_O;
assign i2c_ctrl_rd_en = I2C_STB_I & ~I2C_WE_I & (I2C_ADR_I == I2C_CTRL) & ~I2C_ACK_O;

assign i2c_addr_wr_en = I2C_STB_I & I2C_WE_I  & (I2C_ADR_I == I2C_ADDR) & ~I2C_ACK_O;
assign i2c_addr_rd_en = I2C_STB_I & ~I2C_WE_I & (I2C_ADR_I == I2C_ADDR) & ~I2C_ACK_O;

assign i2c_tx_wr_en   = I2C_STB_I & I2C_WE_I  & (I2C_ADR_I == I2C_TX) & ~I2C_ACK_O;

assign i2c_rx_rd_en   = I2C_STB_I & ~I2C_WE_I & (I2C_ADR_I == I2C_RX) & ~I2C_ACK_O;

assign i2c_dna_wr_en   = I2C_STB_I & I2C_WE_I & (I2C_ADR_I == I2C_DNA) & ~I2C_ACK_O;
assign i2c_dna_rd_en   = I2C_STB_I & ~I2C_WE_I & (I2C_ADR_I == I2C_DNA) & ~I2C_ACK_O;

assign i2c_rbt_wr_en   = I2C_STB_I & I2C_WE_I & (I2C_ADR_I == I2C_RBT) & ~I2C_ACK_O;
assign i2c_rbt_rd_en   = I2C_STB_I & ~I2C_WE_I & (I2C_ADR_I == I2C_RBT) & ~I2C_ACK_O;

always @ (posedge CLK_I or posedge RST_I) begin
	if(RST_I) begin
		reg_rst   <= 1'b0;
		reg_txrst <= 1'b0;
		reg_rxrst <= 1'b0;
	end else if( i2c_ctrl_wr_en ) begin
		reg_rst   <= I2C_DAT_I[23];
		reg_txrst <= I2C_DAT_I[22];
		reg_rxrst <= I2C_DAT_I[21];
	end else begin
		reg_rst   <= 1'b0;
		reg_txrst <= 1'b0;
		reg_rxrst <= 1'b0;
	end
end

reg reg_wstop_r;//a write success
reg reg_rstop_r;//a read success
reg reg_rerr_r ;//a read error

always @ (posedge CLK_I or posedge RST_I) begin
	if(RST_I)
		reg_wstop_r <= 1'b0;
	else if(reg_rst)
		reg_wstop_r <= 1'b0;
	else if(i2c_ctrl_wr_en && I2C_DAT_I[18])
		reg_wstop_r <= 1'b0;
	else if(reg_wstop)
		reg_wstop_r <= 1'b1;
end

always @ (posedge CLK_I or posedge RST_I) begin
	if(RST_I)
		reg_rstop_r <= 1'b0;
	else if(reg_rst)
		reg_rstop_r <= 1'b0;
	else if(i2c_ctrl_wr_en && I2C_DAT_I[19])
		reg_rstop_r <= 1'b0;
	else if(reg_rstop)
		reg_rstop_r <= 1'b1;
end

always @ (posedge CLK_I or posedge RST_I) begin
	if(RST_I)
		reg_rerr_r <= 1'b0;
	else if(reg_rst)
		reg_rerr_r <= 1'b0;
	else if(i2c_ctrl_wr_en && I2C_DAT_I[20])
		reg_rerr_r <= 1'b0;
	else if(reg_rerr)
		reg_rerr_r <= 1'b1;
end

reg reg_rx_mask;

always @ (posedge CLK_I or posedge RST_I) begin
	if(RST_I)
		reg_rx_mask <= 1'b1;
	else if(reg_rst)
		reg_rx_mask <= 1'b1;
	else if(i2c_ctrl_wr_en &&  I2C_DAT_I[24])
		reg_rx_mask <= 1'b1;
	else if(i2c_ctrl_wr_en &&  I2C_DAT_I[25])
		reg_rx_mask <= 1'b0;
end

assign int_i2c = |rx_data_count && ~reg_rx_mask;

always @ (posedge CLK_I or posedge RST_I) begin
	if(RST_I)
		reg_addr <= 7'b0;
	else if(i2c_addr_wr_en)
                reg_addr  <= I2C_DAT_I[6:0];
end

always @ (posedge CLK_I or posedge RST_I) begin
	if(RST_I) begin
		tx_din   <= 32'b0;
		tx_wr_en <= 1'b0;
        end else if(i2c_tx_wr_en) begin
                tx_din <= I2C_DAT_I;
		tx_wr_en <= i2c_tx_wr_en;
	end else
		tx_wr_en <= 1'b0;
end

reg [3:0] reg_dna;
wire dna_dout;
reg reg_download_done;

always @ (posedge CLK_I or posedge RST_I) begin
	if(RST_I)
		reg_dna <= 4'b0;
	else if(i2c_dna_wr_en)
		reg_dna <= I2C_DAT_I[3:0];
end

always @ (posedge CLK_I or posedge RST_I) begin
	if(RST_I)
		I2C_DAT_O <= 32'b0;
	else begin
        	case( 1'b1 )
        	        i2c_ctrl_rd_en  : I2C_DAT_O <= {7'b0, reg_rx_mask, reg_rst, reg_txrst, reg_rxrst, 
        	                                        reg_rerr_r, reg_rstop_r, reg_wstop_r,
        	                                        tx_data_count[8:0], rx_data_count[8:0]};
			i2c_addr_rd_en  : I2C_DAT_O <= {25'b0, reg_addr};
			i2c_rx_rd_en    : I2C_DAT_O <= rx_dout;
			i2c_dna_rd_en   : I2C_DAT_O <= {dna_dout, reg_dna[3:0]};
			i2c_rbt_rd_en   : I2C_DAT_O <= {31'b0, reg_download_done};
        	        default: I2C_DAT_O <= 32'hdeaddead ;
        	endcase
	end
end

i2c_phy i2c_phy(
/*input          */ .clk       (CLK_I        ),
/*input          */ .rst       (RST_I        ),
/*input          */ .reg_rst   (reg_rst      ),
/*input          */ .scl_pin   (scl_pin      ),
/*inout          */ .sda_pin   (sda_pin      ),

/*input  [6:0]   */ .reg_addr  (reg_addr     ),
/*output reg     */ .reg_wstop (reg_wstop    ),//a write success
/*output reg     */ .reg_rstop (reg_rstop    ),//a read success
/*output reg     */ .reg_rerr  (reg_rerr     ),//a read error

/*input          */ .full      (full         ),
/*output reg     */ .push      (rx_wr_en     ),
/*output [31:0]  */ .dout      (rx_din       ),

/*input          */ .empty     (empty        ),
/*output reg     */ .pop       (tx_rd_en     ),
/*input  [31:0]  */ .din       (rbt_enable ? tx_dat : tx_dout      ),

/*output reg [6:0]*/.addr_r    (addr_r),
/*output [7:0] */ .byte_buf    (byte_buf),
/*output       */ .byte_done   (byte_done),

/*output         */ .led_iic_wr(led_iic_wr),
/*output         */ .led_iic_rd(led_iic_rd) 
);

reg brg_cs_r;
assign brg_cs = brg_cs_r;
reg brg_en_r;
assign brg_en = brg_en_r;

always @ (posedge CLK_I or posedge RST_I) begin
	if(RST_I)
		brg_en_r <= 1'b0;
	else if(reg_rst)
		brg_en_r <= 1'b0;
	else if(addr_r == 7'b1000000 && byte_done && byte_buf == 8'h02)
		brg_en_r <= 1'b1;
	else if(addr_r == 7'b1000000 && byte_done && byte_buf == 8'h03)
		brg_en_r <= 1'b0;
end

always @ (posedge CLK_I or posedge RST_I) begin
	if(RST_I)
		brg_cs_r <= 1'b1;
	else if(reg_rst)
		brg_cs_r <= 1'b1;
	else if(addr_r == 7'b1000000 && byte_done && byte_buf == 8'h00)
		brg_cs_r <= 1'b0;
	else if(addr_r == 7'b1000000 && byte_done && byte_buf == 8'h01)
		brg_cs_r <= 1'b1;
end

always @ (posedge CLK_I or posedge RST_I) begin
	if(RST_I)
                reg_download_done <= 1'b0;
        else if(reg_rst)
                reg_download_done <= 1'b0;
        else if(addr_r == 7'b1000000 && byte_done && byte_buf == 8'h04)
                reg_download_done <= 1'b1;
	else if(I2C_DAT_I[0] && i2c_rbt_wr_en)
                reg_download_done <= 1'b0;
end

brg_shift brg_shift(
/*input       */ .clk     (CLK_I),
/*input       */ .rst     (RST_I),
/*input       */ .reg_rst (reg_rst),
/*input       */ .vld     (addr_r == 7'b1000001 && byte_done),
/*input  [7:0]*/ .din     ({byte_buf[0],byte_buf[1],byte_buf[2],byte_buf[3],byte_buf[4],byte_buf[5],byte_buf[6],byte_buf[7]}),
/*output      */ .done    (),

/*output      */ .brg_sck (brg_sck),
/*output      */ .brg_mosi(brg_mosi)
);

i2c_fifo tx_fifo(
/*input          */ .clk       (CLK_I        ),
/*input          */ .srst      (tx_rst       ),
/*input  [31 : 0]*/ .din       (tx_din       ),
/*input          */ .wr_en     (tx_wr_en     ),
/*input          */ .rd_en     (tx_rd_en     ),
/*output [31 : 0]*/ .dout      (tx_dout      ),
/*output         */ .full      (             ),
/*output         */ .empty     (             ),
/*output [8 : 0] */ .data_count(tx_data_count) 
);

i2c_fifo rx_fifo(
/*input          */ .clk       (CLK_I        ),
/*input          */ .srst      (rx_rst       ),
/*input  [31 : 0]*/ .din       (rx_din       ),
/*input          */ .wr_en     (rx_wr_en && ~brg_en    ),
/*input          */ .rd_en     (rx_rd_en     ),
/*output [31 : 0]*/ .dout      (rx_dout      ),
/*output         */ .full      (             ),
/*output         */ .empty     (             ),
/*output [8 : 0] */ .data_count(rx_data_count) 
);

DNA_PORT dna_port(
.DOUT  (dna_dout  ),
.CLK   (reg_dna[0]),
.DIN   (reg_dna[1]),
.READ  (reg_dna[2]),
.SHIFT (reg_dna[3]) 
);
assign tx_dat     = 0;
assign rbt_enable = 0;
assign ram_sel    = 0;//0: iram, 1: dram
assign ram_wr     = 0;
assign ram_addr   = 0;
assign ram_dat_wr = 0;
endmodule
