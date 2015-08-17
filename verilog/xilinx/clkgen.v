module clkgen(input wire clkin, input wire clk25m_on, output wire clkout, output wire [1:0] clk25m, output wire locked, output wire mcu_clk);
//25M,enable,50M,25M,lock
wire clkout_div ;

ODDR2 ODDR2_inst0 (
   .Q (clk25m[0]),   // 1-bit DDR output data
   .C0(clkout_div),   // 1-bit clock input
   .C1(~clkout_div),   // 1-bit clock input
   .CE(clk25m_on),//(1), // 1-bit clock enable input
   .D0(1'b0), // 1-bit data input (associated with C0)
   .D1(1'b1), // 1-bit data input (associated with C1)
   .R (1'b0),   // 1-bit reset input
   .S (1'b0)    // 1-bit set input
);
ODDR2 ODDR2_inst1 (
   .Q (clk25m[1]),   // 1-bit DDR output data
   .C0(clkout_div),   // 1-bit clock input
   .C1(~clkout_div),   // 1-bit clock input
   .CE(clk25m_on),//(1), // 1-bit clock enable input
   .D0(1'b0), // 1-bit data input (associated with C0)
   .D1(1'b1), // 1-bit data input (associated with C1)
   .R (1'b0),   // 1-bit reset input
   .S (1'b0)    // 1-bit set input
);

wire clkfb;
cpm_ctrl5 cpm_ctrl5(
/*input */ .CLK_IN1  (clkin),
/*input */ .CLKFB_IN (clkfb),
/*output*/ .core_clk (clkout),
/*output*/ .chip_clk (clkout_div),
/*output*/ .mcu_clk  (mcu_clk),
/*output*/ .CLKFB_OUT(clkfb),
/*output*/ .LOCKED   (locked)
);

endmodule
