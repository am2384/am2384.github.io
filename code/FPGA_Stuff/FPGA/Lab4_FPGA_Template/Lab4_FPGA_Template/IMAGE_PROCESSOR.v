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

wire white;
wire red;
wire blue;


reg    [1:0] data1 [0:175]; // row number 100

reg    [1:0] data2 [0:175]; // row number 120 



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
		
	end
	
end

always @(posedge CLK) begin
	if(VGA_PIXEL_Y == 100) begin 
		if(red) data[VGA_PIXEL_X] = 2'b01;
		else if(blue) data[VGA_PIXEL_X] = 2'b00;
		else if(white) data[VGA_PIXEL_X] = 2'b10;
		else data[VGA_PIXEL_X] = 2'b11;
	end
		if(VGA_PIXEL_Y == 120) begin 
		if(red) data[VGA_PIXEL_X] = 2'b01;
		else if(blue) data[VGA_PIXEL_X] = 2'b00;
		else if(white) data[VGA_PIXEL_X] = 2'b10;
		else data[VGA_PIXEL_X] = 2'b11;
	end
	
	// 
	
	
	
end





endmodule