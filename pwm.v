



module YS_PWM#(parameter HZ= 50)(  // PWM frequency 50hz = 20ms interval
  input clk, rstn,
  input [7:0] val, // 0~200 range, will use half of servo pwm range(45~135deg), means 1.25ms~1.75ms pulse width, means 60,000~84,000 clk counts. 90deg is 72k clks.
  output out  // PWM output pin
);

reg [32:0] clk_cnt, clks_per_cycle;
reg [16:0] clks_high_dur;  // pwm pulse width(by clocks) : 1ms:0deg,    1.5ms:90deg,      2ms:180deg
reg out_temp;

initial begin
	clks_per_cycle=1000/HZ*48000; //20ms interval = 960k clks
end

always @(posedge clk, negedge rstn) begin

	if (!rstn)	clk_cnt = 0;
	else if (clk_cnt < clks_per_cycle) 	begin
		
		// map = (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
		clks_high_dur = 60000+val*120;  // mapping gamepad value to pwm clock counts.  val : 0~200 range,   will use 45~135deg range mean 60000clk ~ 84000clk   when val:100 72000clk
		
		out = clk_cnt<clks_high_dur ? 1 : 0; // High output if current clock count is under clks_high_dur, otherwise output Low
		
		clk_cnt = clk_cnt+1;
	end
	else clk_cnt=0; // restart cycle
end

endmodule



/*
uint8_t defPadBuf[] = {0xFF, 0xFF, 0x64, 0x64, 0xc8};  // Default Stop PWM generation Frame.
*/

module ReadRF #(parameter CLKS_PER_BIT= 4992, IDLE=300)(  // high bit count to the IDLE times means idle state.  if IDLE=100   104usec*100 = 10msec
  input clk, rstn, in,												 // 
  output out_led,
  output reg [7:0] servo_out,
  output reg [7:0] esc_out
);

reg [9:0] data_all;  // Start_bit + 8bit data + End bit
reg [7:0] data_temp, data[5]; // store 5x 8bit data
reg start_bit, end_bit;

reg [4:0] data_bits_cnt; // count 0~9 for [9:0]data_all
reg [3:0] data_cnt; // count 0~4 for data[5]

reg [16:0] high_cnt; // counting high bits for reset parameter while idle time.

reg [32:0] clk_cnt;  // for clock counting to make 104usec. (9600bps 1 bit duration)

reg in_prev;


initial begin
	in_prev=1;
	data_bits_cnt=0;  // 8N1 10 bit count
	data_cnt=0;			// frame length 5 byte count
	servo_out='h64; esc_out='h64; // Default Stop PWM generation Frame.
end


always @(posedge clk, negedge rstn) begin
	if (!rstn)	clk_cnt = CLKS_PER_BIT;
	else if (clk_cnt==0) 	begin
		clk_cnt=CLKS_PER_BIT; // restart clock count
		
		if(in) begin
			if(high_cnt<10000) high_cnt=high_cnt+1; // 10000(1sec) is max count up. ( prevent back to 0 by adder)
			else begin servo_out='h64; esc_out='h64; data_bits_cnt=0; data_cnt=0; end // if high_cnt over maximum count enfoce stop pwm.
		end
		else high_cnt=0; 
		
		if(high_cnt>IDLE) begin
			data_cnt=0;
			data_bits_cnt=0;
		end
		else begin
			data_all[data_bits_cnt]=in;
			data_bits_cnt=data_bits_cnt+1;
			if(data_bits_cnt>=10) begin
				data_bits_cnt=0;
				{end_bit, data[data_cnt], start_bit} = data_all;
				
				#3 if(start_bit || !end_bit) data[data_cnt]='h64; // start-end bit Error!!! Replace to 0x64  decimal 100. it will put servo 90deg or esc Stop.
				
				data_cnt = data_cnt+1;
				
				#5 if(data_cnt>=5) begin  // receiving frame is finished.
					if( data[0]=='hff && data[1]=='hff && data[2]+data[3]==data[4]) begin // check Start Frame (0xffff) and check sum
						out_led=~out_led; // switch led status if received data OK.
						
						servo_out=data[2]; // pulse width output
						esc_out=data[3]; // pulse width output
					end 
					else begin  
						servo_out='h64; // put stop pwm if frame is broken
						esc_out='h64;
					end
					
					data_cnt=0;
				end
			end
		end
	end // clk_cnt==0
	else begin
		clk_cnt = clk_cnt-1;
		if(!in && high_cnt>IDLE && in!=in_prev) 	// idle중 start_bit를 만나면 ( 104uSec 4992clock의 맨 앞임. ) 안정적인 수신을 위해서 52usec(9600의 한bit의 딱 가운데)에서 lebel 확인을 하도록 cnt를 수정해줌.
			clk_cnt = CLKS_PER_BIT>>1;										// 104usec이라 해도 가끔씩 105일때도 있음. edge에서 lebel확인하는건 아주 안좋은 방법임.
		in_prev<=in;
	end
	
end
endmodule























