`timescale 1ns / 1ps
`include "mm_defines.vh"
module mm(
	// global controls
	input wire CLK,
	// UART 1: FT232
	input wire RXD,
	output wire TXD,
	// RTS and CTS
	// UART 2: debug
	// Avalon ASICs
	// Fan & Temperature control
	// LEDs
	output wire[3:0] LED,
	// SPI Flash
	input wire FLASH_MISO,
	output wire FLASH_MOSI,
	output wire FLASH_SCK,
	output wire FLASH_NCS
);

wire clk_kdf9, reset_kdf9_n;
clkgen clk (.clkin(CLK), .clkout(clk_kdf9), .locked(reset_kdf9_n));

wire [31:0] interrupt_n;
wire [31:0] user_operand_0, user_operand_1, user_result;
wire [10:0] user_opcode;
wire user_valid, user_complete;
superkdf9 cpu ( // {{{
	.clk(clk_kdf9), .rst(~reset_kdf9_n), 
	// active low interrups
	.interrupt_n(interrupt_n),
	// user-defined instruction interface
	.user_opcode(user_opcode),
	.user_operand_0(user_operand_0),
	.user_operand_1(user_operand_1),
	.user_result(user_result),
	.user_valid(user_valid),
	.user_complete(user_complete)
); // }}}
//assign LED = gpio_out[3:0];
assign {FLASH_MOSI, FLASH_SCK, FLASH_NCS, TXD} = 4'b0011;

// test user instructions
//reg user_complete_reg;
//assign user_complete = user_complete_reg;
//always @(posedge clk_kdf9) user_complete_reg <= user_valid;
assign user_complete = user_valid;
assign user_result = user_operand_0 + user_operand_1;

reg [3:0] led_reg;
assign LED = led_reg;
always @(posedge clk_kdf9) begin
	if (user_valid && user_opcode == 11'h1)
		led_reg <= user_operand_0[3:0];
end

// test intr logic
reg [31:0] intr, intr_i_old;
assign interrupt_n = intr;
wire [31:0] intr_i;
always @(posedge clk_kdf9) begin
	if (rst) begin
		intr <= 32'hffff_ffff;
		intr_i_old <= intr_i;
	end else begin
		intr_i_old <= intr_i;
		if (intr_i != intr_i_old) begin
			intr <= intr_i;
		end
		if (user_opcode == 11'hf && user_valid) begin
			intr <= 32'hfff_ffff;
		end
	end
end

// VIO/ILA and ICON {{{
wire [35:0] icon_ctrl_0, icon_ctrl_1;
icon icon_test(.CONTROL0(icon_ctrl_0), .CONTROL1(icon_ctrl_1));
ila ila_test(.CONTROL(icon_ctrl_0), .CLK(clk_kdf9), .TRIG0({
	rst, user_complete, user_valid, user_opcode[4:0]
	,user_operand_0[31:0]
}));
vio vio_test(.CONTROL(icon_ctrl_1), .ASYNC_OUT(intr_i));
// }}}

endmodule
// vim: set fdm=marker : 
