`include "api_define.v"
module api(
input                 CLK_I     ,
input                 RST_I     ,

input                 API_CYC_I ,//NC
input                 API_STB_I ,
input                 API_WE_I  ,
input                 API_LOCK_I,//NC
input  [2:0]          API_CTI_I ,//NC
input  [1:0]          API_BTE_I ,//NC
input  [5:0]          API_ADR_I ,
input  [31:0]         API_DAT_I ,
input  [3:0]          API_SEL_I ,
output                API_ACK_O ,
output                API_ERR_O ,//const 0
output                API_RTY_O ,//const 0
output [31:0]         API_DAT_O ,

output [`API_NUM-1:0] load      ,
output                sck       ,
output                mosi      ,
input  [`API_NUM-1:0] miso      ,

output                led_get_nonce_l,
output                led_get_nonce_h 
);
parameter WORK_LEN = 736/32;
parameter RX_FIFO_DEPTH = 512;

wire [2:0]          reg_state         ;
wire [27:0]         reg_timeout       ;
wire [7:0]          reg_sck           ;
wire [5:0]          reg_ch_num        ;
wire [7:0]          reg_word_num      ;
wire                reg_rst           ;

wire [31 : 0]       tx_fifo_din       ;
wire                tx_fifo_wr_en     ;
wire                tx_fifo_rd_en     ;
wire [31 : 0]       tx_fifo_dout      ;
wire                tx_fifo_full      ;
wire [10 : 0]       tx_fifo_data_count;
wire                tx_fifo_empty = tx_fifo_data_count < reg_word_num;

wire [31 : 0]       rx_fifo_din       ;
wire                rx_fifo_wr_en     ;
wire                rx_fifo_rd_en     ;
wire [31 : 0]       rx_fifo_dout      ;
wire                rx_fifo_full      ;
wire                rx_fifo_empty     ;
wire [9  : 0]       rx_fifo_data_count;

wire clk = CLK_I;
wire rst = RST_I | reg_rst;

wire [3:0] miner_id;
wire [4:0] work_cnt;

api_slave(
// system clock and reset
/*input            */ .clk         (clk               ),
/*input            */ .rst         (RST_I             ),

// wishbone interface signals
/*input            */ .API_CYC_I   (API_CYC_I         ),//NC
/*input            */ .API_STB_I   (API_STB_I         ),
/*input            */ .API_WE_I    (API_WE_I          ),
/*input            */ .API_LOCK_I  (API_LOCK_I        ),//NC
/*input  [2:0]     */ .API_CTI_I   (API_CTI_I         ),//NC
/*input  [1:0]     */ .API_BTE_I   (API_BTE_I         ),//NC
/*input  [5:0]     */ .API_ADR_I   (API_ADR_I         ),
/*input  [31:0]    */ .API_DAT_I   (API_DAT_I         ),
/*input  [3:0]     */ .API_SEL_I   (API_SEL_I         ),//NC
/*output reg       */ .API_ACK_O   (API_ACK_O         ),
/*output           */ .API_ERR_O   (API_ERR_O         ),//const 0
/*output           */ .API_RTY_O   (API_RTY_O         ),//const 0
/*output reg [31:0]*/ .API_DAT_O   (API_DAT_O         ),

/*output reg       */ .txfifo_push (tx_fifo_wr_en     ),
/*output reg [31:0]*/ .txfifo_din  (tx_fifo_din       ),

/*input  [9 :0]    */ .rxcnt       (rx_fifo_data_count),
/*input            */ .rxempty     (rx_fifo_empty     ),
/*input  [10:0]    */ .txcnt       (tx_fifo_data_count),
/*output           */ .reg_flush   (reg_rst           ),
/*input            */ .txfull      (tx_fifo_full      ),

/*input  [2:0]     */ .reg_state   (reg_state         ),
/*output reg [27:0]*/ .reg_timeout (reg_timeout       ),
/*output reg [7:0] */ .reg_sck     (reg_sck           ),
/*output reg [5:0] */ .reg_ch_num  (reg_ch_num        ),
/*output reg [7:0] */ .reg_word_num(reg_word_num      ),

/*input            */ .rx_fifo_wr_en(rx_fifo_wr_en),
/*input  [31:0]    */ .rx_fifo_din  (rx_fifo_din  ),
/*input  [3:0]     */ .miner_id     (miner_id     ),
/*input  [4:0]     */ .work_cnt     (work_cnt     ),

/*output           */ .rxfifo_pop  (rx_fifo_rd_en     ),
/*input  [31:0]    */ .rxfifo_dout (rx_fifo_dout      )  
);


//-------------------------------------------------
// WBBUS
//-------------------------------------------------
api_ctrl #(.WORK_LEN(WORK_LEN), .RX_FIFO_DEPTH(RX_FIFO_DEPTH)) api_ctrl(
/*input                */ .clk               (clk               ),
/*input                */ .rst               (rst               ),

/*output [2:0]         */ .reg_state         (reg_state         ),
/*input  [27:0]        */ .reg_timeout       (reg_timeout       ),
/*input  [7:0]         */ .reg_sck           (reg_sck           ),
/*input  reg [5:0]     */ .reg_ch_num        (reg_ch_num        ),
/*input  reg [7:0]     */ .reg_word_num      (reg_word_num      ),

/*input                */ .tx_fifo_empty     (tx_fifo_empty     ),
/*output               */ .tx_fifo_rd_en     (tx_fifo_rd_en     ),
/*input  [31:0]        */ .tx_fifo_dout      (tx_fifo_dout      ),

/*output               */ .rx_fifo_wr_en     (rx_fifo_wr_en     ),
/*output [31:0]        */ .rx_fifo_din       (rx_fifo_din       ),
/*input  [9  : 0]      */ .rx_fifo_data_count(rx_fifo_data_count),
/*output [3:0]         */ .miner_id          (miner_id          ),
/*output reg [4:0]     */ .work_cnt          (work_cnt          ),

/*output [`API_NUM-1:0]*/ .load              (load              ),
/*output               */ .sck               (sck               ),
/*output               */ .mosi              (mosi              ),
/*input  [`API_NUM-1:0]*/ .miso              (miso              ),

/*output               */ .led_get_nonce_l   (led_get_nonce_l   ),
/*output               */ .led_get_nonce_h   (led_get_nonce_h   ) 
);


//1024words
fifo1024 tx_fifo(
/*input          */ .clk       (clk               ),
/*input          */ .srst      (rst               ),
/*input  [31 : 0]*/ .din       (tx_fifo_din       ),
/*input          */ .wr_en     (tx_fifo_wr_en     ),
/*input          */ .rd_en     (tx_fifo_rd_en     ),
/*output [31 : 0]*/ .dout      (tx_fifo_dout      ),
/*output         */ .full      (tx_fifo_full      ),
/*output         */ .empty     (                  ),
/*output [10 : 0]*/ .data_count(tx_fifo_data_count)
) ;                                       

//512words
fifo512 rx_fifo(                          
/*input          */ .clk       (clk               ),
/*input          */ .srst      (rst               ),
/*input  [31 : 0]*/ .din       (rx_fifo_din       ),
/*input          */ .wr_en     (rx_fifo_wr_en     ),
/*input          */ .rd_en     (rx_fifo_rd_en     ),
/*output [31 : 0]*/ .dout      (rx_fifo_dout      ),
/*output         */ .full      (rx_fifo_full      ),
/*output         */ .empty     (rx_fifo_empty     ),
/*output [9  : 0]*/ .data_count(rx_fifo_data_count)
);
/*
wire [35:0] icon_ctrl_0;
wire [255:0] trig0 = {
miso,//6:5
mosi,//4
sck,//3
load,//2:1
API_CYC_I
} ;
icon icon_test(.CONTROL0(icon_ctrl_0));
ila ila_test(.CONTROL(icon_ctrl_0), .CLK(clk), .TRIG0(trig0)
);
*/

endmodule
