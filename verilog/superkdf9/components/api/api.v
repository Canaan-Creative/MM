`include "api_define.v"
module api(
input                 CLK_I          ,
input                 RST_I          ,

input                 API_CYC_I      ,//NC
input                 API_STB_I      ,
input                 API_WE_I       ,
input                 API_LOCK_I     ,//NC
input  [2:0]          API_CTI_I      ,//NC
input  [1:0]          API_BTE_I      ,//NC
input  [5:0]          API_ADR_I      ,
input  [31:0]         API_DAT_I      ,
input  [3:0]          API_SEL_I      ,
output                API_ACK_O      ,
output                API_ERR_O      ,//const 0
output                API_RTY_O      ,//const 0
output [31:0]         API_DAT_O      ,

output [`API_NUM-1:0] load           ,
output                sck            ,
output                mosi           ,
input  [`API_NUM-1:0] miso           ,

output                led_get_nonce_l,
output                led_get_nonce_h,
output                api_idle 
);
parameter WORK_LEN = 736/32;
parameter RX_FIFO_DEPTH = 512;

wire [2:0]          reg_state         ;
wire [27:0]         reg_timeout       ;
wire [7:0]          reg_sck           ;
wire [5:0]          reg_ch_num        ;
wire [8:0]          reg_word_num      ;
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
wire                rx_fifo_empty     ;
wire [9  : 0]       rx_fifo_data_count;

wire clk = CLK_I;
wire rst = RST_I;

wire [3:0] miner_id;
wire [4:0] work_cnt;

wire          reg_pllf_rst       ;
wire [103: 0] reg_pllf_data      ;
wire          reg_pllf_wr_en     ;
wire          pllf_rd_en         ;
wire [103: 0] pllf_dout          ;
wire          reg_pllf_full      ;
wire          reg_pllf_empty     ;
wire [6 : 0]  reg_pllf_data_count;

api_slave api_slave(
// system clock and reset
/*input            */ .clk                (clk                ),
/*input            */ .rst                (RST_I              ),

// wishbone interface signals
/*input            */ .API_CYC_I          (API_CYC_I          ),//NC
/*input            */ .API_STB_I          (API_STB_I          ),
/*input            */ .API_WE_I           (API_WE_I           ),
/*input            */ .API_LOCK_I         (API_LOCK_I         ),//NC
/*input  [2:0]     */ .API_CTI_I          (API_CTI_I          ),//NC
/*input  [1:0]     */ .API_BTE_I          (API_BTE_I          ),//NC
/*input  [5:0]     */ .API_ADR_I          (API_ADR_I          ),
/*input  [31:0]    */ .API_DAT_I          (API_DAT_I          ),
/*input  [3:0]     */ .API_SEL_I          (API_SEL_I          ),//NC
/*output reg       */ .API_ACK_O          (API_ACK_O          ),
/*output           */ .API_ERR_O          (API_ERR_O          ),//const 0
/*output           */ .API_RTY_O          (API_RTY_O          ),//const 0
/*output reg [31:0]*/ .API_DAT_O          (API_DAT_O          ),

/*output reg       */ .txfifo_push        (tx_fifo_wr_en      ),
/*output reg [31:0]*/ .txfifo_din         (tx_fifo_din        ),

/*input  [9 :0]    */ .rxcnt              (rx_fifo_data_count ),
/*input            */ .rxempty            (rx_fifo_empty      ),
/*input  [10:0]    */ .txcnt              (tx_fifo_data_count ),
/*output           */ .reg_flush          (reg_rst            ),
/*input            */ .txfull             (tx_fifo_full       ),

/*input  [2:0]     */ .reg_state          (reg_state          ),
/*output reg [27:0]*/ .reg_timeout        (reg_timeout        ),
/*output reg [7:0] */ .reg_sck            (reg_sck            ),
/*output reg [5:0] */ .reg_ch_num         (reg_ch_num         ),
/*output reg [8:0] */ .reg_word_num       (reg_word_num       ),

/*input            */ .rx_fifo_wr_en      (rx_fifo_wr_en      ),
/*input  [31:0]    */ .rx_fifo_din        (rx_fifo_din        ),
/*input  [3:0]     */ .miner_id           (miner_id           ),
/*input  [4:0]     */ .work_cnt           (work_cnt           ),

/*output           */ .rxfifo_pop         (rx_fifo_rd_en      ),
/*input  [31:0]    */ .rxfifo_dout        (rx_fifo_dout       ), 

/*output reg       */ .reg_pllf_rst       (reg_pllf_rst       ),
/*output reg[103:0]*/ .reg_pllf_data      (reg_pllf_data      ),
/*output reg       */ .reg_pllf_wr_en     (reg_pllf_wr_en     ),
/*input            */ .reg_pllf_full      (reg_pllf_full      ),
/*input            */ .reg_pllf_empty     (reg_pllf_empty     ),
/*input [6 : 0]    */ .reg_pllf_data_count(reg_pllf_data_count) 
);


//-------------------------------------------------
// WBBUS
//-------------------------------------------------
api_ctrl #(.WORK_LEN(WORK_LEN), .RX_FIFO_DEPTH(RX_FIFO_DEPTH)) api_ctrl(
/*input                */ .clk               (clk               ),
/*input                */ .rst               (rst               ),

/*input                */ .reg_rst           (reg_rst           ),
/*output [2:0]         */ .reg_state         (reg_state         ),
/*input  [27:0]        */ .reg_timeout       (reg_timeout       ),
/*input  [7:0]         */ .reg_sck           (reg_sck           ),
/*input  reg [5:0]     */ .reg_ch_num        (reg_ch_num        ),
/*input  reg [8:0]     */ .reg_word_num      (reg_word_num      ),
/*output               */ .timeout_busy      (timeout_busy      ),

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
/*output               */ .led_get_nonce_h   (led_get_nonce_h   ),

/*output               */ .pllf_rd_en        (pllf_rd_en        ),
/*input                */ .reg_pllf_empty    (reg_pllf_empty    ),
/*input  [103:0]       */ .pllf_dout         (pllf_dout         )
);

assign api_idle = reg_state == 3'b0 && tx_fifo_empty && ~timeout_busy;

//1024words
fifo1024 tx_fifo(
/*input          */ .clk       (clk               ),
/*input          */ .srst      (rst | reg_rst     ),
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
/*input          */ .srst      (rst | reg_rst     ),
/*input  [31 : 0]*/ .din       (rx_fifo_din       ),
/*input          */ .wr_en     (rx_fifo_wr_en     ),
/*input          */ .rd_en     (rx_fifo_rd_en     ),
/*output [31 : 0]*/ .dout      (rx_fifo_dout      ),
/*output         */ .full      (                  ),
/*output         */ .empty     (rx_fifo_empty     ),
/*output [9  : 0]*/ .data_count(rx_fifo_data_count)
);

assign pllf_dout           = 0;
assign reg_pllf_full       = 0;
assign reg_pllf_empty      = 1;
assign reg_pllf_data_count = 0;

endmodule
