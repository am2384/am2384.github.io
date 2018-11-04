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
reg [7:0]	pixel_data_RGB332;
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
wire [3:0] RESULT;

/* WRITE ENABLE */
reg W_EN;
wire c0_sig;
wire c1_sig;
wire c2_sig;

// ALL CONNECTIONS HERE
wire [7:0] camera; 
wire PCLK, HREF, VSYNC;

assign camera = {GPIO_1_D[33],GPIO_1_D[32],GPIO_1_D[31],GPIO_1_D[30],GPIO_1_D[29],GPIO_1_D[28],GPIO_1_D[27],GPIO_1_D[26]};
assign GPIO_0_D[0] = c0_sig; // Camera team needs the clock at pin number 2 on bank 1 
assign PCLK = GPIO_1_D[23];
assign HREF = GPIO_1_D[24];
assign VSYNC = GPIO_1_D[25];



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
	.RESULT({GPIO_0_D[14],GPIO_0_D[12],GPIO_0_D[10],GPIO_0_D[8]})
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

reg [7:0] fake_camera;
/////* Update Write Address *///////
always @ (posedge PCLK) begin
		
		//W_EN = 1;
//		
//		if(WRITE_ADDRESS < 144 * 176) begin
//			WRITE_ADDRESS <= WRITE_ADDRESS + 1;
//		end
//		else begin
//			WRITE_ADDRESS <= WRITE_ADDRESS;
//		end
		
		
		if(WRITE_ADDRESS % 176 < 20) begin
			fake_camera = WHITE;
		end
		else if(WRITE_ADDRESS % 176 < 40) begin
			fake_camera = LIGHTBLUE;
		end
		else if(WRITE_ADDRESS % 176 < 60) begin
			fake_camera = YELLOW;
		end
		else if (WRITE_ADDRESS % 176 < 80) begin
			fake_camera = GREEN;
		end
		else if (WRITE_ADDRESS % 176 < 100) begin
			fake_camera = PURPLE;
		end
		else if (WRITE_ADDRESS % 176 < 120) begin
			fake_camera = BLUE;
		end
		else if (WRITE_ADDRESS % 176 < 140) begin
			fake_camera = BROWN;
		end
		else begin
			fake_camera = BLACK;
		end 
		
end


// input to pixel data

//wire start_toggle;
reg prev_HREF;
wire res;
reg toggle;
assign res = prev_HREF==1 && HREF==0;
reg [14:0] row_counter;
reg [14:0] col_counter;

always @ (posedge PCLK) begin 
	
		
//		if(~toggle) begin
//			pixel_data_RGB332[7] = camera[7]; // RED
//			pixel_data_RGB332[6] = camera[6]; // RED
//			pixel_data_RGB332[5] = camera[5]; // RED
//			pixel_data_RGB332[4] = camera[2]; // GREEN
//			pixel_data_RGB332[3] = camera[1]; // GREEN
//			pixel_data_RGB332[2] = camera[0]; // GREEN
//			pixel_data_RGB332[1] = pixel_data_RGB332[1]; // available next clock cycle 
//			pixel_data_RGB332[0] = pixel_data_RGB332[0]; // available next clock cycle
//		end
//		else begin
//			pixel_data_RGB332[1] = camera[4]; // BLUE 
//			pixel_data_RGB332[0] = camera[3]; // BLUE
//			pixel_data_RGB332[2] = pixel_data_RGB332[2]; // BLUE
//			pixel_data_RGB332[7] = pixel_data_RGB332[7]; // from previous clock cycle
//			pixel_data_RGB332[6] = pixel_data_RGB332[6]; // from previous clock cycle
//			pixel_data_RGB332[5] = pixel_data_RGB332[5]; // from previous clock cycle
//			pixel_data_RGB332[4] = pixel_data_RGB332[4]; // from previous clock cycle
//			pixel_data_RGB332[3] = pixel_data_RGB332[3]; // from previous clock cycle
//		end

		if(~toggle) begin
			pixel_data_RGB332[7] = fake_camera[7]; // RED
			pixel_data_RGB332[6] = fake_camera[6]; // RED
			pixel_data_RGB332[5] = fake_camera[5]; // RED
			pixel_data_RGB332[4] = fake_camera[2]; // GREEN
			pixel_data_RGB332[3] = fake_camera[1]; // GREEN
			pixel_data_RGB332[2] = fake_camera[0]; // GREEN
			pixel_data_RGB332[1] = pixel_data_RGB332[1]; // available next clock cycle 
			pixel_data_RGB332[0] = pixel_data_RGB332[0]; // available next clock cycle
		end
		else begin
			pixel_data_RGB332[1] = fake_camera[4]; // BLUE 
			pixel_data_RGB332[0] = fake_camera[3]; // BLUE
			pixel_data_RGB332[2] = pixel_data_RGB332[2]; // BLUE
			pixel_data_RGB332[7] = pixel_data_RGB332[7]; // from previous clock cycle
			pixel_data_RGB332[6] = pixel_data_RGB332[6]; // from previous clock cycle
			pixel_data_RGB332[5] = pixel_data_RGB332[5]; // from previous clock cycle
			pixel_data_RGB332[4] = pixel_data_RGB332[4]; // from previous clock cycle
			pixel_data_RGB332[3] = pixel_data_RGB332[3]; // from previous clock cycle
		end
		
		// toggle  
		// ~ toggle
      // ~toggle
		
		// ~toggle  
		// toggle
      // toggle
		// 
		
		
		
		W_EN = toggle;
		
		if(HREF) toggle = ~toggle;  
		else toggle =0;
	
	// Timing /////////////////////
	
	// first  CC: HREF = 1, toggle = 0->1 , W_EN = 0->0, first byte read,  row_counter=0, col_counter = 0->0
	// second CC: HREF = 1, toggle = 1->0 , W_EN = 0->1, second byte read, row_counter=0, col_counter = 0->0      CHECK
	// third  CC: HREF = 1, toggle = 0->1 , W_EN = 1->0, first byte read,  row_counter=0, col_counter = 0->1
	// fourth CC: HREF = 1, toggle = 1->0 , W_EN = 0->1, second byte read, row_counter=0, col_counter = 1->1      CHECK
	// fifth  CC: HREF = 1, toggle = 0->1 , W_EN = 1->0, first byte read,  row_counter=0, col_counter = 1->2
	// sixth  CC: HREF = 1, toggle = 1->0 , W_EN = 0->1, second byte read, row_counter=0, col_counter = 2->2      CHECK
	// 7th    CC: HREF = 1, toggle = 0->1 , W_EN = 1->0, first byte read,  row_counter=0, col_counter = 2->3
	// 8th    CC: HREF = 1, toggle = 1->0 , W_EN = 0->1, second byte read, row_counter=0, col_counter = 3->3      CHECK
	
		prev_HREF = HREF; 
	
		if(VSYNC) begin 
			WRITE_ADDRESS = 0;
		   row_counter = 0;
		   col_counter = 0;
		end
		else if(W_EN && HREF) begin 
			if (col_counter == 175) col_counter = col_counter;
		   else col_counter = col_counter + 1;   
			WRITE_ADDRESS = row_counter * 176 + col_counter;
		end
		else if(res) begin // negative edge of HREF
		   col_counter = 0;
			if (row_counter == 143) row_counter = row_counter;
			else row_counter = row_counter + 1;
		end
		else begin
			WRITE_ADDRESS = WRITE_ADDRESS;
			col_counter = col_counter;
			row_counter = row_counter;
		end
      
		
			
end



	
endmodule 
