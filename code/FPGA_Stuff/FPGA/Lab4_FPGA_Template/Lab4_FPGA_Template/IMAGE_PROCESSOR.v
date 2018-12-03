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


reg [18:0] red_count;
reg [18:0] blue_count;
reg prev_VGA_VSYNC_NEG;
wire negege_VGA_VSYNC_NEG;

reg [9:0] l1;
reg [9:0] l2;
reg [9:0] l3;

assign white = PIXEL_IN[7] & PIXEL_IN[4] & PIXEL_IN[2];
assign blue = (PIXEL_IN[1] == 0) & ~ white;
assign red =  (PIXEL_IN[1]) & ~white;

assign negege_VGA_VSYNC_NEG = prev_VGA_VSYNC_NEG & ~VGA_VSYNC_NEG;

always @(posedge CLK) begin
	prev_VGA_VSYNC_NEG <= VGA_VSYNC_NEG;
	
	if(negege_VGA_VSYNC_NEG) begin
		RESULT[0] <= (red_count > blue_count); // UPDATE COLOR WHEN A FRAME ENDS  CHANGE THAT!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		RESULT[1] <= ((red_count + blue_count) > 3/5*176 * 144); // 0 for WHITE => 0 means absence of treasure
		if((l1 <  (l2-20)) && (l2 <  (l3-20))) RESULT[3:2] <= 2'b00; // triangle
		else if((l1 < (l2-5)) && ((l2-5) > l3)) RESULT[3:2] <= 2'b01; // diamond
		else RESULT[3:2] <= 2'b10; // square  
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
		l1 = 0;
		l2 = 0;
		l3 = 0;
	end
	else begin
		if (red && VGA_READ_MEM_EN) red_count <= red_count + 1'b1;
		else red_count <= red_count;
		if (blue && VGA_READ_MEM_EN) blue_count <= blue_count + 1'b1;
		else blue_count <= blue_count;
	end

	// Shape Detection Code
	if(VGA_VSYNC_NEG && VGA_READ_MEM_EN) begin
	
		if(VGA_PIXEL_Y== 36) begin
			if((red || blue)) begin
				l1 <= l1 + 1;
			end
		end
		else if(VGA_PIXEL_Y == 72) begin
			if((red || blue)) begin
				l2 <= l2 + 1;
			end
		end
		else if(VGA_PIXEL_Y == 108) begin
			if((red || blue)) begin
				l3 <= l3 + 1;
			end
		end
		else begin
			l1 <= l1;
			l2 <= l2;
			l3 <= l3;
		end
	end
	else begin
		l1 <= l1;
		l2 <= l2;
		l3 <= l3;
	end
end





assign white = PIXEL_IN[7] & PIXEL_IN[4] & PIXEL_IN[2];
assign blue = (PIXEL_IN[1] == 0) & ~ white;
assign red =  (PIXEL_IN[1]) & ~white;

assign negege_VGA_VSYNC_NEG = prev_VGA_VSYNC_NEG & ~VGA_VSYNC_NEG;

always @(posedge CLK) begin
	prev_VGA_VSYNC_NEG <= VGA_VSYNC_NEG;
	
	// output the results when a frame ends
	if(negege_VGA_VSYNC_NEG) begin
		RESULT[0] <= (red_count > blue_count); 
		// HIGH when color = red, LOW for color = blue 
		RESULT[1] <= ((red_count + blue_count) > 3/5*176 * 144); 
		// 1 = Treasure present, 0 = No Treasure
		if((l1 <  (l2-20)) && (l2 <  (l3-20))) RESULT[3:2] <= 2'b00; // shape = triangle
		else if((l1 < (l2-5)) && ((l2-5) > l3)) RESULT[3:2] <= 2'b01; // shape = diamond
		else RESULT[3:2] <= 2'b10; // shape = square  
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
		l1 = 0;
		l2 = 0;
		l3 = 0;
	end
	else begin
		if (red && VGA_READ_MEM_EN) red_count <= red_count + 1;
		else red_count <= red_count;
		if (blue && VGA_READ_MEM_EN) blue_count <= blue_count + 1;
		else blue_count <= blue_count;
	end

	// Shape Detection Code
	if(VGA_VSYNC_NEG && VGA_READ_MEM_EN) begin
	
		if(VGA_PIXEL_Y== 36) begin
			if((red || blue)) begin
				l1 <= l1 + 1;
			end
		end
		else if(VGA_PIXEL_Y == 72) begin
			if((red || blue)) begin
				l2 <= l2 + 1;
			end
		end
		else if(VGA_PIXEL_Y == 108) begin
			if((red || blue)) begin
				l3 <= l3 + 1;
			end
		end
		else begin
			l1 <= l1;
			l2 <= l2;
			l3 <= l3;
		end
	end
	else begin
		l1 <= l1;
		l2 <= l2;
		l3 <= l3;
	end
end

















endmodule
