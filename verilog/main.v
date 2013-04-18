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

wire[31:0] interrupt_n;
superkdf9 cpu (.clk(clk_kdf9), .rst(~reset_kdf9_n) /*, .interrupt_n(interrupt_n)*/);
//assign LED = gpio_out[3:0];
assign {FLASH_MOSI, FLASH_SCK, FLASH_NCS, TXD} = 4'b0011;

reg[31:0] count;
reg led;
assign LED = {4{led}};
always @(posedge clk_kdf9) begin
	count <= count + 1;
	if (count == 32'd50_000_000) begin
		count <= 0;
		led <= ~led;
	end
end

endmodule
// vim: set fdm=marker : 
