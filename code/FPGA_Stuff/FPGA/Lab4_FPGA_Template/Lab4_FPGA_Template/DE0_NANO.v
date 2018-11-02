`define SCREEN_WIDTH 176
`define SCREEN_HEIGHT 144

///////* DON'T CHANGE THIS PART *///////
module DE0_NANO(
	CLOCK_50,
	GPIO_0_D,
	GPIO_1_D,
	KEY
);

//=======================================================
//  PARAMETER declarations
//=======================================================
localparam RED = 8'b111_000_00;
localparam GREEN = 8'b000_111_00;
localparam BLUE = 8'b000_000_11;
localparam BLACK = 8'b000_000_00;
localparam WHITE = 8'b111_111_11;
localparam YELLOW = 8'b111_111_00;
localparam PURPLE = 8'b111_000_11;
localparam LIGHTBLUE = 8'b000_010_11;
localparam BROWN = 8'b010_001_00;

//=======================================================
//  PORT declarations
//=======================================================

//////////// CLOCK - DON'T NEED TO CHANGE THIS //////////
input 		          		CLOCK_50;

//////////// GPIO_0, GPIO_0 connect to GPIO Default //////////
output 		    [33:0]		GPIO_0_D;
//////////// GPIO_0, GPIO_1 connect to GPIO Default //////////
input 		    [33:20]		GPIO_1_D;
input 		     [1:0]		KEY;

///// PIXEL DATA /////
reg [7:0]	pixel_data_RGB332 = WHITE;
//The adapter takes pixel data in RGB 332 format (8 bits - 3 red, 3 green, 2 blue).
///// READ/WRITE ADDRESS /////
reg [14:0] X_ADDR;
reg [14:0] Y_ADDR;
reg [14:0] WRITE_ADDRESS;
reg [14:0] READ_ADDRESS; 

//assign WRITE_ADDRESS = X_ADDR + Y_ADDR*(`SCREEN_WIDTH);

///// VGA INPUTS/OUTPUTS /////
wire 			VGA_RESET;
wire [7:0]	VGA_COLOR_IN;
wire [9:0]	VGA_PIXEL_X;
wire [9:0]	VGA_PIXEL_Y;
wire [7:0]	MEM_OUTPUT;
wire			VGA_VSYNC_NEG;
wire			VGA_HSYNC_NEG;
reg			VGA_READ_MEM_EN;

assign GPIO_0_D[5] = VGA_VSYNC_NEG;
assign VGA_RESET = ~KEY[0];

///// I/O for Img Proc /////
wire [8:0] RESULT;

/* WRITE ENABLE */
reg W_EN;
wire c0_sig;
wire c1_sig;
wire c2_sig;

//////////////////////// CAMERA INPUTS ////////////////////////////////
wire [7:0] camera; 
wire PCLK, HREF, VSYNC;
assign camera = {GPIO_1_D[33],GPIO_1_D[32],GPIO_1_D[31],GPIO_1_D[30],GPIO_1_D[29],GPIO_1_D[28],GPIO_1_D[27],GPIO_1_D[26]};
assign GPIO_0_D[0] = c0_sig; // Camera team needs the clock at pin numer 2 on bank 1 
assign PCLK = GPIO_1_D[23];
assign HREF = GPIO_1_D[24];
assign VSYNC = GPIO_1_D[25];

// assign camera = CAMERA_PIXEL_INPUT (will assign GPIO Pins later)


//////////////////////////////////////////////////////////////////////////

///////* CREATE ANY LOCAL WIRES YOU NEED FOR YOUR PLL *///////


///////* INSTANTIATE YOUR PLL HERE *///////
PLL	pll_inst (
	.inclk0 (CLOCK_50 ),
	.c0 ( c0_sig ),  //24 MHz
	.c1 ( c1_sig ), // 25 MhZ
	.c2 ( c2_sig ) // 50 MHz
	);

///////* M9K Module *///////
Dual_Port_RAM_M9K mem(
	.input_data(pixel_data_RGB332),
	.w_addr(WRITE_ADDRESS),
	.r_addr(READ_ADDRESS),
	.w_en(W_EN),
	.clk_W(CLOCK_50), // write frequency = 50 MHz
	.clk_R(c1_sig ), // read frequency = 25 MHz
	.output_data(MEM_OUTPUT)
);



///////* VGA Module *///////
VGA_DRIVER driver (
	.RESET(VGA_RESET),
	.CLOCK(c1_sig ), 
	.PIXEL_COLOR_IN(VGA_READ_MEM_EN ? MEM_OUTPUT : RED),
	.PIXEL_X(VGA_PIXEL_X),
	.PIXEL_Y(VGA_PIXEL_Y),
	.PIXEL_COLOR_OUT({GPIO_0_D[9],GPIO_0_D[11],GPIO_0_D[13],GPIO_0_D[15],GPIO_0_D[17],GPIO_0_D[19],GPIO_0_D[21],GPIO_0_D[23]}),
   .H_SYNC_NEG(GPIO_0_D[7]),
   .V_SYNC_NEG(VGA_VSYNC_NEG)
);

///////* Image Processor *///////
IMAGE_PROCESSOR proc(
	.PIXEL_IN(MEM_OUTPUT),
	.CLK(c1_sig ),
	.VGA_PIXEL_X(VGA_PIXEL_X),
	.VGA_PIXEL_Y(VGA_PIXEL_Y),
	.VGA_VSYNC_NEG(VGA_VSYNC_NEG),
	.RESULT(RESULT)
);


///////* Update Read Address *///////
always @ (VGA_PIXEL_X, VGA_PIXEL_Y) begin
		READ_ADDRESS = (VGA_PIXEL_X + VGA_PIXEL_Y*`SCREEN_WIDTH);
		if(VGA_PIXEL_X>(`SCREEN_WIDTH-1) || VGA_PIXEL_Y>(`SCREEN_HEIGHT-1))begin
				VGA_READ_MEM_EN = 1'b0;
		end
		else begin
				VGA_READ_MEM_EN = 1'b1;
		end
end

///////* Update Write Address *///////
//always @ (posedge c2_sig) begin
//		
//		W_EN = 1;
//		
//		if(WRITE_ADDRESS < 144 * 176) begin
//			WRITE_ADDRESS <= WRITE_ADDRESS + 1;
//		end
//		else begin
//			WRITE_ADDRESS <= WRITE_ADDRESS;
//		end
//		
//		
//		if(WRITE_ADDRESS % 176 < 20) begin
//			pixel_data_RGB332 = WHITE;
//		end
//		else if(WRITE_ADDRESS % 176 < 40) begin
//			pixel_data_RGB332 = LIGHTBLUE;
//		end
//		else if(WRITE_ADDRESS % 176 < 60) begin
//			pixel_data_RGB332 = YELLOW;
//		end
//		else if (WRITE_ADDRESS % 176 < 80) begin
//			pixel_data_RGB332 = GREEN;
//		end
//		else if (WRITE_ADDRESS % 176 < 100) begin
//			pixel_data_RGB332 = PURPLE;
//		end
//		else if (WRITE_ADDRESS % 176 < 120) begin
//			pixel_data_RGB332 = BLUE;
//		end
//		else if (WRITE_ADDRESS % 176 < 140) begin
//			pixel_data_RGB332 = BROWN;
//		end
//		else begin
//			pixel_data_RGB332 = BLACK;
//		end 
//		
//end


// input to pixel data

reg toggle;
reg prev_VSYNC;
wire start_frame;

assign start_frame = (prev_VSYNC==1) && (VSYNC==0);

always @ (posedge PCLK) begin // replace with PCLK later  
	
	
		prev_VSYNC = VSYNC;
		if(start_frame) WRITE_ADDRESS = 0;
		
		//RGB565  -> RGB 332

		
		if(~toggle) begin
			pixel_data_RGB332[7] = camera[7]; // RED
			pixel_data_RGB332[6] = camera[6]; // RED
			pixel_data_RGB332[5] = camera[5]; // RED
			pixel_data_RGB332[4] = camera[2]; // green
			pixel_data_RGB332[3] = camera[1]; // GREEN
			pixel_data_RGB332[2] = camera[0]; // GREEN
			pixel_data_RGB332[1] = 1'b0; // GREEN
			pixel_data_RGB332[0] = 1'b0; // GREEN
			
		end
		else begin
			pixel_data_RGB332[1] = camera[4]; // blue 
			pixel_data_RGB332[0] = camera[3]; // blue 
			pixel_data_RGB332[7] = pixel_data_RGB332[7]; // RED
			pixel_data_RGB332[6] = pixel_data_RGB332[6]; // RED
			pixel_data_RGB332[5] = pixel_data_RGB332[5]; // RED
			pixel_data_RGB332[4] = pixel_data_RGB332[4]; // green
			pixel_data_RGB332[3] = pixel_data_RGB332[3]; // GREEN
			pixel_data_RGB332[2] = pixel_data_RGB332[2];
		end

		W_EN = toggle;
		
		if(toggle && HREF) begin
			WRITE_ADDRESS = WRITE_ADDRESS +1;
			toggle = ~toggle;
		end
		else begin
			WRITE_ADDRESS = WRITE_ADDRESS;
		end
		
		
		
end



	
endmodule 