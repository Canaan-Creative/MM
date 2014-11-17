module mboot(
input             CLK_I       ,
input             RST_I       ,

input             MBOOT_CYC_I ,//NC
input             MBOOT_STB_I ,
input             MBOOT_WE_I  ,
input             MBOOT_LOCK_I,//NC
input  [2:0]      MBOOT_CTI_I ,//NC
input  [1:0]      MBOOT_BTE_I ,//NC
input  [5:0]      MBOOT_ADR_I ,
input  [31:0]     MBOOT_DAT_I ,
input  [3:0]      MBOOT_SEL_I ,
output reg        MBOOT_ACK_O ,
output            MBOOT_ERR_O ,//const 0
output            MBOOT_RTY_O ,//const 0
output reg [31:0] MBOOT_DAT_O ,

output reg        MBOOT_SCL   ,
output reg        MBOOT_CS    ,
output reg        MBOOT_MOSI  ,
output reg        MBOOT_HOLD_N,
output reg        MBOOT_WP_N  ,
input             MBOOT_MISO   
);

assign MBOOT_ERR_O = 1'b0 ;
assign MBOOT_RTY_O = 1'b0 ;

`define MBOOT_FLASH 6'h0
`define MBOOT_ICAPO 6'h4
`define MBOOT_ICAPI 6'h8

//-----------------------------------------------------
// WB bus ACK
//-----------------------------------------------------
always @ ( posedge CLK_I or posedge RST_I ) begin
	if( RST_I )
		MBOOT_ACK_O <= 1'b0 ;
	else if( MBOOT_STB_I && (~MBOOT_ACK_O) )
		MBOOT_ACK_O <= 1'b1 ;
	else
		MBOOT_ACK_O <= 1'b0 ;
end

wire flash_wr_en  = MBOOT_STB_I &  MBOOT_WE_I & (MBOOT_ADR_I == `MBOOT_FLASH) & ~MBOOT_ACK_O ;
wire flash_rd_en  = MBOOT_STB_I & ~MBOOT_WE_I & (MBOOT_ADR_I == `MBOOT_FLASH) & ~MBOOT_ACK_O ;

wire icapo_wr_en  = MBOOT_STB_I &  MBOOT_WE_I & (MBOOT_ADR_I == `MBOOT_ICAPO) & ~MBOOT_ACK_O ;
wire icapo_rd_en  = MBOOT_STB_I & ~MBOOT_WE_I & (MBOOT_ADR_I == `MBOOT_ICAPO) & ~MBOOT_ACK_O ;

wire icapri_rd_en = MBOOT_STB_I & ~MBOOT_WE_I & (MBOOT_ADR_I == `MBOOT_ICAPI) & ~MBOOT_ACK_O ;

always @ (posedge CLK_I or posedge RST_I) begin
	if(RST_I)
		{MBOOT_SCL, MBOOT_CS, MBOOT_MOSI, MBOOT_HOLD_N, MBOOT_WP_N} <= 5'b01011;
	else if(flash_wr_en)
		{MBOOT_SCL, MBOOT_CS, MBOOT_MOSI, MBOOT_HOLD_N, MBOOT_WP_N} <= MBOOT_DAT_I[4:0];
end

wire        ICAP_BUSY ;
wire [15:0] ICAP_O    ;
reg         ICAP_CLK  ;
reg         ICAP_CE   ;
reg         ICAP_WRITE;
reg  [15:0] ICAP_I    ;

always @ (posedge CLK_I or posedge RST_I) begin
	if(RST_I)
		{ICAP_CLK, ICAP_CE, ICAP_WRITE, ICAP_I[15:0]} <= 19'h30000;
	else if(icapo_wr_en)
		{ICAP_CLK, ICAP_CE, ICAP_WRITE, ICAP_I[15:0]} <= MBOOT_DAT_I[18:0];
end

ICAP_SPARTAN6 ICAP_SPARTAN6(
/*output       */ .BUSY (ICAP_BUSY ),
/*output [15:0]*/ .O    (ICAP_O    ),

/*input        */ .CLK  (ICAP_CLK  ),
/*input        */ .CE   (ICAP_CE   ),
/*input        */ .WRITE(ICAP_WRITE),
/*input  [15:0]*/ .I    ({ICAP_I[8],ICAP_I[9],ICAP_I[10],ICAP_I[11],ICAP_I[12],ICAP_I[13],ICAP_I[14],ICAP_I[15], ICAP_I[0],ICAP_I[1],ICAP_I[2],ICAP_I[3],ICAP_I[4],ICAP_I[5],ICAP_I[6],ICAP_I[7]})
);

always @ (posedge CLK_I) begin
	MBOOT_DAT_O <=  flash_rd_en ? {24'h123456, 2'b0, MBOOT_MISO, MBOOT_SCL, MBOOT_CS, MBOOT_MOSI, MBOOT_HOLD_N, MBOOT_WP_N}:
			icapo_rd_en ? {13'b0, ICAP_CLK, ICAP_CE, ICAP_WRITE, ICAP_I[15:0]}: {15'b0, ICAP_BUSY, ICAP_O[15:0]};
end
/*
wire [35 : 0] CONTROL;
wire [79 : 0] TRIG0 = {
flash_wr_en,//40
flash_rd_en,//39
icapo_wr_en,//38
icapo_rd_en,//37
icapri_rd_en,//36
ICAP_I[15:0],//35:20
ICAP_WRITE  ,//19
ICAP_CE     ,//18
ICAP_CLK    ,//17
ICAP_O[15:0],//16:1
ICAP_BUSY    //0
};
ila ila(
 .CONTROL(CONTROL),
 .CLK    (CLK_I  ),
 .TRIG0  (TRIG0  )
);

icon icon(
 .CONTROL0(CONTROL)
);
*/
endmodule
