`define SCREEN_WIDTH 176
`define SCREEN_HEIGHT 144
`define NUM_BARS 3
`define BAR_HEIGHT 48



module IMAGE_PROCESSOR (
	PIXEL_IN,
	CLK,
	VGA_PIXEL_X,
	VGA_PIXEL_Y,
	VGA_VSYNC_NEG,
	VGA_READ_MEM_EN,
	RESULT
);
	



//=======================================================
//  PORT declarations
//=======================================================
input	[7:0]	PIXEL_IN;
input 		CLK;

input [9:0] VGA_PIXEL_X;
input [9:0] VGA_PIXEL_Y;
input			VGA_VSYNC_NEG;
input       VGA_READ_MEM_EN;
output [3:0] RESULT;
reg [3:0] RESULT;

wire white;
wire red;
wire blue;
////////////////////////////////

////////////////////////
// you’ll need to create a system to pass information 
//(about treasure presence, color, and shape) from the 
//FPGA to the Arduino using the digital ports on both of these devices.
//  RESULT[0] = color 
//  RESULT[1] = presence
//  RESULT[3:2] = shape (2'b00 for square, 2'b01 for triangle, 2'b10 for rhombus)
// VGA_PIXEL_X varies from 0 to 175
// VGA_PIXEL_Y varies from 0 to 143 

//assign white = (PIXEL_IN[7]+PIXEL_IN[6]+PIXEL_IN[5]+PIXEL_IN[4]+PIXEL_IN[3]+PIXEL_IN[2]+PIXEL_IN[1]+PIXEL_IN[0]) >= 6; // mostly white. 
//assign red =  (PIXEL_IN[7]==1 || (PIXEL_IN[6]==1 && PIXEL_IN[5]==1) ) && PIXEL_IN[2]==0;
//assign blue = PIXEL_IN[7]==0 && ((PIXEL_IN[2]==1) || (PIXEL_IN[1]==1 && PIXEL_IN[0]==1) );
//assign red  = (PIXEL_IN[7:6] > PIXEL_IN[1:0]);
//assign blue = (PIXEL_IN[7:6] < PIXEL_IN[1:0]);


//assign white = PIXEL_IN[7] & PIXEL_IN[4] & PIXEL_IN[2];
//assign black = (PIXEL_IN[1] == 0) & ~ white;
//assign blue =  (PIXEL_IN[1]) & ~white;


// RGB332 => RRR_GGG_BB
assign red = (PIXEL_IN[7]==1 || (PIXEL_IN[6]==1 && PIXEL_IN[5]==1) ) && PIXEL_IN[2]==0;
assign blue = PIXEL_IN[7]==0 && ( PIXEL_IN[1]==1 );
assign white = (PIXEL_IN[7]==1 && PIXEL_IN[4]==1 && PIXEL_IN[1]==1) || (~red && ~blue);  



reg [18:0] red_count;
reg [18:0] blue_count;
reg prev_VGA_VSYNC_NEG;
wire negege_VGA_VSYNC_NEG;
reg t1;
reg [9:0] x1;
reg t2;
reg [9:0] x2;
reg t3;
reg [9:0] x3;
reg t4;
reg [9:0] x4;
reg t5;
reg [9:0] x5;
reg t6;
reg [9:0] x6;

assign negege_VGA_VSYNC_NEG = prev_VGA_VSYNC_NEG & ~VGA_VSYNC_NEG;

/////////////////////////////////////////////////// PREVIOUS CODE ////////////

/* prev_VGA_VSYNC_NEG = VGA_VSYNC_NEG;
	
	if(negege_VGA_VSYNC_NEG) begin
		RESULT[0] = (red_count > blue_count); // UPDATE COLOR WHEN A FRAME ENDS
		RESULT[1] = red_count + blue_count > 3/5*176 * 144; // 0 for WHITE
	end
	else begin
		RESULT[0] = RESULT[0];
		RESULT[1] = RESULT[1];
	end
	
	if(~VGA_VSYNC_NEG) begin
		red_count = 0;
		blue_count = 0;
	end
	else begin
		if (blue && VGA_READ_MEM_EN) red_count = red_count + 1'b1;
		else red_count = red_count;
		if (black && VGA_READ_MEM_EN) blue_count = blue_count + 1'b1;
		else blue_count = blue_count;
	end */
	//RESULT[0] = red;
//////////////////////////////////////////////////////////////////////////////

always @(posedge CLK) begin
	prev_VGA_VSYNC_NEG <= VGA_VSYNC_NEG;
	
	if(negege_VGA_VSYNC_NEG) begin
		RESULT[0] <= (red_count > blue_count); // UPDATE COLOR WHEN A FRAME ENDS
		RESULT[1] <= ((red_count + blue_count) > 3/5*176 * 144); // 0 for WHITE => 0 means absence of treasure
		if(x1> x2 && x2 > x3 && x3 > x4 && x4 > x5 && x5 > x6) RESULT[3:2] <= 2'b00; // triangle
		else if((x1 > x2 && x2 > x3) && (x6 > x5 && x5 > x4)) RESULT[3:2] <= 2'b01; // diamond
		else RESULT[3:2] <= 2'b10; // square  
		t1 <= t1;
		t2 <= t2;
		t3 <= t3;
		t4 <= t4;
		t5 <= t5;
		t6 <= t6;
		
	end
	else begin
		RESULT[0] <= RESULT[0];
		RESULT[1] <= RESULT[1];
		RESULT[3:2] <= RESULT[3:2];
	end
	
	// Color Detection Code 
	if(~VGA_VSYNC_NEG) begin
		red_count <= 0;
		blue_count <= 0;
	end
	else begin
		if (red && VGA_READ_MEM_EN) red_count <= red_count + 1'b1;
		else red_count <= red_count;
		if (blue && VGA_READ_MEM_EN) blue_count <= blue_count + 1'b1;
		else blue_count <= blue_count;
	end



	// Shape Detection Code
	if(VGA_VSYNC_NEG && VGA_READ_MEM_EN) begin
	
		if(VGA_PIXEL_Y== 45) begin
			if((red || blue) && t1) begin
					x1 <= VGA_PIXEL_X;
					t1 <= ~t1;
			end
		end
		else if(VGA_PIXEL_Y == 50) begin
			if((red || blue) && t2) begin
				x2 <= VGA_PIXEL_X;
				t2 <= ~t2;
			end
		end
		else if(VGA_PIXEL_Y == 55) begin
			if((red || blue) && t3) begin
				x3 <= VGA_PIXEL_X;
				t3 <= ~t3;
			end
		end
		else if(VGA_PIXEL_Y == 86) begin
			if((red || blue) && t4) begin
				x4 <= VGA_PIXEL_X;
				t4 <= ~x4;
			end
		end
		else if(VGA_PIXEL_Y == 91) begin
			if((red || blue) && t5) begin
				x5 <= VGA_PIXEL_X;
				t5 <= ~t5;
			end
		end
		else if(VGA_PIXEL_Y == 96) begin
			if((red || blue) && t6) begin
				x6 <= VGA_PIXEL_X;
				t6 <= ~t6;
			end
		end
		else begin
			x1 <= x1;
			x2 <= x2;
			x3 <= x3;
			x4 <= x4;
			x5 <= x5;
			x6 <= x6;
			t1 <= t1;
			t2 <= t2;
			t3 <= t3;
			t4 <= t4;
			t5 <= t5;
			t6 <= t6;
		end
	end
	else begin
		x1 <= x1;
		x2 <= x2;
		x3 <= x3;
		x4 <= x4;
		x5 <= x5;
		x6 <= x6;
		t1 <= t1;
		t2 <= t2;
		t3 <= t3;
		t4 <= t4;
		t5 <= t5;
		t6 <= t6;
	end
end


endmodule
