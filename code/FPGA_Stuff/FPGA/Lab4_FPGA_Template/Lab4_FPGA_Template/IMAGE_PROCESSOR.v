`define SCREEN_WIDTH 176
`define SCREEN_HEIGHT 144
`define NUM_BARS 3
`define BAR_HEIGHT 48

localparam R = 2'b01;
localparam B = 2'b00;
localparam W = 2'b10;

module IMAGE_PROCESSOR (
	PIXEL_IN,
	CLK,
	VGA_PIXEL_X,
	VGA_PIXEL_Y,
	VGA_VSYNC_NEG,
	RESULT
);

// C0 = (87, 71),
//C1 = (43, 35), 
//C2 = (131, 35), 
//C3 = (43, 107),
//C4 = (131, 107), 



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
////////////////////////////////
reg [1:0] c0;
reg [1:0] c1;
reg [1:0] c2;
reg [1:0] c3;
reg [1:0] c4;

////////////////////////
// you’ll need to create a system to pass information 
//(about treasure presence, color, and shape) from the 
//FPGA to the Arduino using the digital ports on both of these devices.
//  RESULT[0] = color 
//  RESULT[1] = presence
//  RESULT[3:2] = shape (2'b00 for square, 2'b01 for triangle, 2'b10 for rhombus)
// VGA_PIXEL_X varies from 0 to 175
// VGA_PIXEL_Y varies from 0 to 143 

assign white = (PIXEL_IN[7]+PIXEL_IN[6]+PIXEL_IN[5]+PIXEL_IN[4]+PIXEL_IN[3]+PIXEL_IN[2]+PIXEL_IN[1]+PIXEL_IN[0]) >= 6; // mostly white. 
assign red =  PIXEL_IN[7]==1 && PIXEL_IN[2]==0;
assign blue = PIXEL_IN[7]==0 && PIXEL_IN[2]==1;

always @(posedge CLK) begin
	if(~VGA_VSYNC_NEG) begin
		RESULT[0] = (c0 == 2'b01);
		RESULT[1] = (c0 == 2'b11);
		if(c1== R && c2==R && c3==R && c4==R) RESULT[3:2]= 2'b00;
		else if(c1== W && c2==W && c3==R && c4==R) RESULT[3:2]= 2'b01;
		else if(c1== W && c2==W && c3==W && c4==W) RESULT[3:2]= 2'b10;
		else RESULT[3:2] = RESULT[3:2];
	end
	
end


//‘Red’    =  2’b01
//‘Blue’    = 2’b00
//‘White’  = 2’b10
//‘Other’  = 2’b11    

always @(posedge CLK) begin
	if(VGA_PIXEL_Y == 35) begin   // c1
		if(VGA_PIXEL_X== 43) begin
			if(red) c1 = 2'b01;
			else if(blue) c1 = 2'b00;
			else if(white) c1 = 2'b10;
			else c1 = 2'b11;
		end
		else if(VGA_PIXEL_X== 131)  // c2
			if(red) c2 = 2'b01;
			else if(blue) c2 = 2'b00;
			else if(white) c2 = 2'b10;
			else c2 = 2'b11;
		end
	end
	else if(VGA_PIXEL_Y == 107) begin  // c3
		if(VGA_PIXEL_X== 43) begin
			if(red) c3 = 2'b01;
			else if(blue) c3 = 2'b00;
			else if(white) c3 = 2'b10;
			else c3 = 2'b11;
		end
		else if(VGA_PIXEL_X == 131)  // c4
			if(red) c4 = 2'b01;
			else if(blue) c4 = 2'b00;
			else if(white) c4 = 2'b10;
			else c4 = 2'b11;
		end
	end 
	else if(VGA_PIXEL_Y == 71) begin // c0
		if(VGA_PIXEL_X== 87) begin
			if(red) c0 = 2'b01;
			else if(blue) c0 = 2'b00;
			else if(white) c0 = 2'b10;
			else c0 = 2'b11;
		end
	end
	
	
	// C0 = (87, 71),
//C1 = (43, 35), 
//C2 = (131, 35), 
//C3 = (43, 107),
//C4 = (131, 107), 
	
	
	
end





endmodule