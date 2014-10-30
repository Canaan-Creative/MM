module led_ctrl(
input         clk       ,
input         rst       ,
input         vld       ,
input  [31:0] reg_din   ,//stable
input  [7:0]  reg_breath,

input  [3:0]  led_bling ,

output        sft_shcp  ,
output        sft_ds     
);

parameter LED_OFF    = 4'b0000;
parameter LED_ON     = 4'b0001;
parameter LED_SPARK  = 4'b0010;
parameter LED_SPARK1 = 4'b0011;
parameter LED_BLING  = 4'b0100;
parameter LED_BREATH = 4'b0101;

parameter SPARK_WIH = 25;
parameter SPARK_MAX = 25'h989680 * 3;//300ms
parameter BLING_MAX = 25'h1;

parameter BREATH_MAX  = 25'h180000;// 1/60s

reg [SPARK_WIH-1:0] led0_spark_cnt;
reg [SPARK_WIH-1:0] led1_spark_cnt;
reg [SPARK_WIH-1:0] led2_spark_cnt;
reg [SPARK_WIH-1:0] led3_spark_cnt;
reg [SPARK_WIH-1:0] led4_spark_cnt;
reg [SPARK_WIH-1:0] led5_spark_cnt;
reg [SPARK_WIH-1:0] led6_spark_cnt;
reg [SPARK_WIH-1:0] led7_spark_cnt;

wire [3:0] led0 = reg_din[ 3: 0];
wire [3:0] led1 = reg_din[ 7: 4];
wire [3:0] led2 = reg_din[11: 8];
wire [3:0] led3 = reg_din[15:12];
wire [3:0] led4 = reg_din[19:16];
wire [3:0] led5 = reg_din[23:20];
wire [3:0] led6 = reg_din[27:24];
wire [3:0] led7 = reg_din[31:28];

reg [7:0] led7_breath_cnt;
reg [7:0] led7_breath_round_cnt;
reg led7_breath_flg;
always @ (posedge clk) begin
	if(rst || (vld && (led7 == LED_OFF || led7 == LED_ON)))
		led7_breath_cnt <= 0;
	else if(led_spark_cnt_max[7] && led7_breath_cnt != reg_breath)
		led7_breath_cnt <= led7_breath_cnt + 1;
	else if(led_spark_cnt_max[7])
		led7_breath_cnt <= 0;
end

always @ (posedge clk) begin
	if(rst || (vld && (led7 == LED_OFF || led7 == LED_ON)))
		led7_breath_round_cnt <= 0;
	else if(led_spark_cnt_max[7] && led7_breath_cnt == reg_breath && led7_breath_round_cnt != reg_breath)
		led7_breath_round_cnt <= led7_breath_round_cnt + 1;
	else if(led_spark_cnt_max[7] && led7_breath_cnt == reg_breath)
		led7_breath_round_cnt <= 0;
end

always @ (posedge clk) begin
	if(rst || (vld && (led7 == LED_OFF || led7 == LED_ON)))
		led7_breath_flg <= 0;
	else if(led_spark_cnt_max[7] && led7_breath_cnt == reg_breath && led7_breath_round_cnt == reg_breath)
		led7_breath_flg <= ~led7_breath_flg;
end

reg vld_r;

wire [7:0] led_spark_cnt_max;
assign led_spark_cnt_max[0] = led0_spark_cnt == SPARK_MAX;
assign led_spark_cnt_max[1] = led1_spark_cnt == SPARK_MAX;
assign led_spark_cnt_max[2] = led2 == LED_BLING ? led2_spark_cnt == BLING_MAX:led2_spark_cnt == SPARK_MAX;
assign led_spark_cnt_max[3] = led3 == LED_BLING ? led3_spark_cnt == BLING_MAX:led3_spark_cnt == SPARK_MAX;
assign led_spark_cnt_max[4] = led4 == LED_BLING ? led4_spark_cnt == BLING_MAX:led4_spark_cnt == SPARK_MAX;
assign led_spark_cnt_max[5] = led5 == LED_BLING ? led5_spark_cnt == BLING_MAX:led5_spark_cnt == SPARK_MAX;
assign led_spark_cnt_max[6] = led6_spark_cnt == SPARK_MAX;
assign led_spark_cnt_max[7] = led7 == LED_BREATH ? led7_spark_cnt == BREATH_MAX : led7_spark_cnt == SPARK_MAX;

reg [7:0] led_r;
wire led7_w, led6_w, led5_w, led4_w, led3_w, led2_w, led1_w, led0_w;
wire [7:0] led_tmp;
assign led_tmp[6:0] = led_r[6:0] ^ led_spark_cnt_max[6:0];
assign led_tmp[7] =     (led7 == LED_BREATH && (~led7_breath_flg))? (led7_breath_cnt >= led7_breath_round_cnt):
			(led7 == LED_BREATH                      )? (led7_breath_cnt < led7_breath_round_cnt) :
                        (led_r[7] ^ led_spark_cnt_max[7]);

always @ (posedge clk) begin
	if(rst)
		led_r <= 8'h0;
	else if(vld)
		led_r <= {led7_w, led6_w, led5_w, led4_w, led3_w, led2_w, led1_w, led0_w};
	else if(|led_spark_cnt_max)
		led_r <= led_tmp;
end

always @ (posedge clk) begin
	if(rst)
		vld_r <= 1'b0;
	else if(vld || (|led_spark_cnt_max))
		vld_r <= 1'b1;
	else
		vld_r <= 1'b0;
end

reg [31:0] reg_din_r;
always @ (posedge clk) begin
	reg_din_r <= reg_din;
end
wire led0_change = reg_din_r[ 3: 0] == reg_din[ 3: 0] ? 1'b0 : 1'b1;
wire led1_change = reg_din_r[ 7: 4] == reg_din[ 7: 4] ? 1'b0 : 1'b1;
wire led2_change = reg_din_r[11: 8] == reg_din[11: 8] ? 1'b0 : 1'b1;
wire led3_change = reg_din_r[15:12] == reg_din[15:12] ? 1'b0 : 1'b1;
wire led4_change = reg_din_r[19:16] == reg_din[19:16] ? 1'b0 : 1'b1;
wire led5_change = reg_din_r[23:20] == reg_din[23:20] ? 1'b0 : 1'b1;
wire led6_change = reg_din_r[27:24] == reg_din[27:24] ? 1'b0 : 1'b1;
wire led7_change = reg_din_r[31:28] == reg_din[31:28] ? 1'b0 : 1'b1;

assign led0_w = (led0 == LED_OFF) ? 1'b0 : (led0 == LED_ON) ? 1'b1 : (led0 == LED_SPARK) ? 1'b1 : led_r[0];
assign led1_w = (led1 == LED_OFF) ? 1'b0 : (led1 == LED_ON) ? 1'b1 : (led1 == LED_SPARK) ? 1'b1 : led_r[1];
assign led2_w = (led2 == LED_OFF) ? 1'b0 : (led2 == LED_ON) ? 1'b1 : (led2 == LED_SPARK) ? 1'b1 : led_r[2];
assign led3_w = (led3 == LED_OFF) ? 1'b0 : (led3 == LED_ON) ? 1'b1 : (led3 == LED_SPARK) ? 1'b1 : led_r[3];
assign led4_w = (led4 == LED_OFF) ? 1'b0 : (led4 == LED_ON) ? 1'b1 : (led4 == LED_SPARK) ? 1'b1 : led_r[4];
assign led5_w = (led5 == LED_OFF) ? 1'b0 : (led5 == LED_ON) ? 1'b1 : (led5 == LED_SPARK) ? 1'b1 : led_r[5];
assign led6_w = (led6 == LED_OFF) ? 1'b0 : (led6 == LED_ON) ? 1'b1 : (led6 == LED_SPARK) ? 1'b1 : led_r[6];
assign led7_w = (led7 == LED_OFF) ? 1'b0 : (led7 == LED_ON) ? 1'b1 : (led7 == LED_SPARK) ? 1'b1 : led_r[7];

always @ (posedge clk) begin
	if(rst)
		led0_spark_cnt <= 0;
	else if(vld && (led0 == LED_OFF || led0 == LED_ON))
		led0_spark_cnt <= 0;
	else if(vld && (led0 == LED_SPARK || (led0 == LED_SPARK1 && led0_change)))
		led0_spark_cnt <= 1;
	else if(led_spark_cnt_max[0] && led0 == LED_SPARK)
		led0_spark_cnt <= 0;
	else if(led_spark_cnt_max[0] && led0 == LED_SPARK1)
		led0_spark_cnt <= 1;
	else if(|led0_spark_cnt)
		led0_spark_cnt <= led0_spark_cnt + 1;
end

always @ (posedge clk) begin
        if(rst)
                led1_spark_cnt <= 0;                                                                              
       else if(vld && (led1 == LED_OFF || led1 == LED_ON))                                                       
                led1_spark_cnt <= 0;
	else if(vld && (led1 == LED_SPARK || (led1 == LED_SPARK1 && led1_change)))
                led1_spark_cnt <= 1;                                                                              
        else if(led_spark_cnt_max[1] && led1 == LED_SPARK)
                led1_spark_cnt <= 0;
        else if(led_spark_cnt_max[1] && led1 == LED_SPARK1)
                led1_spark_cnt <= 1;
        else if(|led1_spark_cnt)
                led1_spark_cnt <= led1_spark_cnt + 1;
end


always @ (posedge clk) begin
	if(rst)
		led2_spark_cnt <= 0;
       else if(vld && (led2 == LED_OFF || led2 == LED_ON))                                                       
                led2_spark_cnt <= 0;
	else if(vld && (led2 == LED_SPARK || ((led2 == LED_SPARK1 || led2 == LED_BLING) && led2_change)))
		led2_spark_cnt <= 1;
	else if(led_spark_cnt_max[2] && led2 == LED_SPARK)
		led2_spark_cnt <= 0;
	else if(led_spark_cnt_max[2] && (led2 == LED_SPARK1 || led2 == LED_BLING))
		led2_spark_cnt <= 1;
	else if(|led2_spark_cnt)
		led2_spark_cnt <= led2 == LED_BLING ? led2_spark_cnt + led_bling[0]: led2_spark_cnt + 1;
end

always @ (posedge clk) begin
        if(rst)
                led3_spark_cnt <= 0;
       else if(vld && (led3 == LED_OFF || led3 == LED_ON))                                                       
                led3_spark_cnt <= 0;
	else if(vld && (led3 == LED_SPARK || ((led3 == LED_SPARK1 || led3 == LED_BLING) && led3_change)))
                led3_spark_cnt <= 1;
        else if(led_spark_cnt_max[3] && led3 == LED_SPARK)
                led3_spark_cnt <= 0;
        else if(led_spark_cnt_max[3] && (led3 == LED_SPARK1 || led3 == LED_BLING))
                led3_spark_cnt <= 1;
        else if(|led3_spark_cnt)
                led3_spark_cnt <= led3 == LED_BLING ? led3_spark_cnt + led_bling[1]: led3_spark_cnt + 1;
end

always @ (posedge clk) begin
        if(rst)
                led4_spark_cnt <= 0;
       else if(vld && (led4 == LED_OFF || led4 == LED_ON))                                                       
                led4_spark_cnt <= 0;
	else if(vld && (led4 == LED_SPARK || ((led4 == LED_SPARK1 || led4 == LED_BLING) && led4_change)))
                led4_spark_cnt <= 1;
        else if(led_spark_cnt_max[4] && led4 == LED_SPARK)
                led4_spark_cnt <= 0;
        else if(led_spark_cnt_max[4] && (led4 == LED_SPARK1 || led4 == LED_BLING))
                led4_spark_cnt <= 1;
        else if(|led4_spark_cnt)
                led4_spark_cnt <= led4 == LED_BLING ? led4_spark_cnt + led_bling[2]: led4_spark_cnt + 1;
end

always @ (posedge clk) begin
        if(rst)
                led5_spark_cnt <= 0;
       else if(vld && (led5 == LED_OFF || led5 == LED_ON))                                                       
                led5_spark_cnt <= 0;
	else if(vld && (led5 == LED_SPARK || ((led5 == LED_SPARK1 || led5 == LED_BLING) && led5_change)))
                led5_spark_cnt <= 1;
        else if(led_spark_cnt_max[5] && led5 == LED_SPARK)
                led5_spark_cnt <= 0;
        else if(led_spark_cnt_max[5] && (led5 == LED_SPARK1 || led5 == LED_BLING))
                led5_spark_cnt <= 1;
        else if(|led5_spark_cnt)
                led5_spark_cnt <= led5 == LED_BLING ? led5_spark_cnt + led_bling[3]: led5_spark_cnt + 1;
end

always @ (posedge clk) begin
        if(rst)
                led6_spark_cnt <= 0;      
       else if(vld && (led6 == LED_OFF || led6 == LED_ON))                                                       
                led6_spark_cnt <= 0;                                                                        
	else if(vld && (led6 == LED_SPARK || (led6 == LED_SPARK1 && led6_change)))
                led6_spark_cnt <= 1;                                                                              
        else if(led_spark_cnt_max[6] && led6 == LED_SPARK)
                led6_spark_cnt <= 0;
        else if(led_spark_cnt_max[6] && led6 == LED_SPARK1)
                led6_spark_cnt <= 1;
        else if(|led6_spark_cnt)
                led6_spark_cnt <= led6_spark_cnt + 1;
end

always @ (posedge clk) begin
        if(rst)
                led7_spark_cnt <= 0;
       else if(vld && (led7 == LED_OFF || led7 == LED_ON))                                                       
                led7_spark_cnt <= 0;                                                                              
	else if(vld && (led7 == LED_SPARK || (led7 == LED_SPARK1 && led7_change) || (led7 == LED_BREATH && led7_change)))
                led7_spark_cnt <= 1;                                                                              
        else if(led_spark_cnt_max[7] && led7 == LED_SPARK)
                led7_spark_cnt <= 0;
        else if(led_spark_cnt_max[7] && (led7 == LED_SPARK1 || led7 == LED_BREATH))
                led7_spark_cnt <= 1;
        else if(|led7_spark_cnt)
                led7_spark_cnt <= led7_spark_cnt + 1;
end


led_shift led_shift(
/*input       */ .clk     (clk     ),
/*input       */ .rst     (rst     ),
/*input       */ .vld     (vld_r   ),
/*input  [7:0]*/ .din     (led_r   ),
/*output      */ .done    (        ),

/*output      */ .sft_shcp(sft_shcp),
/*output      */ .sft_ds  (sft_ds  ) 
);

endmodule
