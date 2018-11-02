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

output [8:0] RESULT;

reg redC, blueC;
wire white;
wire red;
wire blue;

// you’ll need to create a system to pass information 
//(about treasure presence, color, and shape) from the 
//FPGA to the Arduino using the digital ports on both of these devices.
//  RESULT[0] = color 
//  RESULT[1] = presence
//  RESULT[3:2] = shape
// VGA_PIXEL_X varies from 0 to 175
// VGA_PIXEL_Y varies from 0 to 143 

assign white = (PIXEL_IN[7]+PIXEL_IN[6]+PIXEL_IN[5]+PIXEL_IN[4]+PIXEL_IN[3]+PIXEL_IN[2]+PIXEL_IN[1]+PIXEL_IN[0]) >= 6; // mostly white. 
assign red = PIXEL_IN[7]==1 && PIXEL_IN[2]==0;
assign blue = PIXEL_IN[7]==0 && PIXEL_IN[2]==1;

always @(posedge CLK) begin
	if(~VGA_VSYNC_NEG) begin
		if(redC > blueC) RESULT[0] = 1;
		else RESULT[0] = 0;
		redC = 0;
		blueC = 0;
	end
	
end

always @(posedge CLK) begin
	if(VGA_PIXEL_X)
	if(red) redC = redC + 1; // detect red or blue 
	else blueC = blueC + 1;
	
	if(white)      // 
	
end





endmodule