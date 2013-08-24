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
DCM_CLKGEN #( // {{{
	.CLKFXDV_DIVIDE(2), // CLKFXDV divide value (2, 4, 8, 16, 32)
	.CLKFX_DIVIDE(16), // Divide value - D - (1-256)
	.CLKFX_MD_MAX(0.0), // Specify maximum M/D ratio for timing anlysis
	.CLKFX_MULTIPLY(50), // Multiply value - M - (2-256)
	.CLKIN_PERIOD(30.0), // Input clock period specified in nS
	.SPREAD_SPECTRUM("NONE"), // Spread Spectrum mode "NONE", "CENTER_LOW_SPREAD", "CENTER_HIGH_SPREAD",
	// "VIDEO_LINK_M0", "VIDEO_LINK_M1" or "VIDEO_LINK_M2"
	.STARTUP_WAIT("FALSE") // Delay config DONE until DCM_CLKGEN LOCKED (TRUE/FALSE)
) DCM_CLKGEN_kdf9 (
	.CLKFX(clk_kdf9), // 1-bit output: Generated clock output
	.CLKFX180(), // 1-bit output: Generated clock output 180 degree out of phase from CLKFX.
	.CLKFXDV(), // 1-bit output: Divided clock output
	.LOCKED(reset_kdf9_n), // 1-bit output: Locked output
	.PROGDONE(), // 1-bit output: Active high output to indicate the successful re-programming
	.STATUS(), // 2-bit output: DCM_CLKGEN status
	.CLKIN(CLK), // 1-bit input: Input clock
	.FREEZEDCM(1'b0), // 1-bit input: Prevents frequency adjustments to input clock
	.PROGCLK(1'b0), // 1-bit input: Clock input for M/D reconfiguration
	.PROGDATA(1'b0), // 1-bit input: Serial data input for M/D reconfiguration
	.PROGEN(1'b0), // 1-bit input: Active high program enable
	.RST(1'b0) // 1-bit input: Reset input pin
); // }}}

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
