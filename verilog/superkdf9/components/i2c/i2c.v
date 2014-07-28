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

output                int_i2c
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

wire rst = reg_rst | RST_I;

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

wire          full         = rx_data_count + PKG_LEN >= 256;
wire          empty        = tx_data_count < PKG_LEN;

wire dna_done;
wire [56:0] dna;

parameter I2C_CTRL = 6'h00;
parameter I2C_ADDR = 6'h04;
parameter I2C_TX   = 6'h08;
parameter I2C_RX   = 6'h0c;
parameter I2C_DNA  = 6'h10;

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

always @ (posedge CLK_I) begin
        if( i2c_ctrl_wr_en ) begin
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

always @ (posedge CLK_I) begin
	if(rst)
		reg_wstop_r <= 1'b0;
	else if(i2c_ctrl_wr_en && I2C_DAT_I[18])
		reg_wstop_r <= 1'b0;
	else if(reg_wstop)
		reg_wstop_r <= 1'b1;
end

always @ (posedge CLK_I) begin
	if(rst)
		reg_rstop_r <= 1'b0;
	else if(i2c_ctrl_wr_en && I2C_DAT_I[19])
		reg_rstop_r <= 1'b0;
	else if(reg_rstop)
		reg_rstop_r <= 1'b1;
end

always @ (posedge CLK_I) begin
	if(rst)
		reg_rerr_r <= 1'b0;
	else if(i2c_ctrl_wr_en && I2C_DAT_I[20])
		reg_rerr_r <= 1'b0;
	else if(reg_rerr)
		reg_rerr_r <= 1'b1;
end

reg reg_rx_mask;

always @ (posedge CLK_I) begin
	if(rst)
		reg_rx_mask <= 1'b1;
	else if(i2c_ctrl_wr_en &&  I2C_DAT_I[24])
		reg_rx_mask <= 1'b1;
	else if(i2c_ctrl_wr_en &&  I2C_DAT_I[25])
		reg_rx_mask <= 1'b0;
end

assign int_i2c = |rx_data_count && ~reg_rx_mask;

always @ (posedge CLK_I) begin
	if(RST_I)
		reg_addr <= 7'b0;
        else if(i2c_addr_wr_en)
                reg_addr  <= I2C_DAT_I[6:0];
end

always @ (posedge CLK_I) begin
        if(i2c_tx_wr_en) begin
                tx_din <= I2C_DAT_I;
		tx_wr_en <= i2c_tx_wr_en;
	end else
		tx_wr_en <= 1'b0;
end

reg [3:0] reg_dna;
wire dna_dout;

always @ (posedge CLK_I) begin
	if(RST_I)
		reg_dna <= 4'b0;
	else if(i2c_dna_wr_en)
		reg_dna <= I2C_DAT_I[3:0];
end

always @ (posedge CLK_I) begin
        case( 1'b1 )
                i2c_ctrl_rd_en  : I2C_DAT_O <= {7'b0, reg_rx_mask, reg_rst, reg_txrst, reg_rxrst, 
                                                reg_rerr_r, reg_rstop_r, reg_wstop_r,
                                                tx_data_count[8:0], rx_data_count[8:0]};
		i2c_addr_rd_en  : I2C_DAT_O <= {25'b0, reg_addr};
		i2c_rx_rd_en    : I2C_DAT_O <= rx_dout;
		i2c_dna_rd_en   : I2C_DAT_O <= {dna_dout, reg_dna[3:0]};
                default: I2C_DAT_O <= 32'hdeaddead ;
        endcase
end

i2c_phy i2c_phy(
/*input          */ .clk       (CLK_I        ),
/*input          */ .rst       (rst          ),
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
/*input  [31:0]  */ .din       (tx_dout      )
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
/*input          */ .wr_en     (rx_wr_en     ),
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

endmodule
