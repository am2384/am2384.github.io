/*
 Copyright (C) 2011 J. Coliz <maniacbug@ymail.com>
 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */

/**
 * Example for Getting Started with nRF24L01+ radios.
 *
 * This is an example of how to use the RF24 class.  Write this sketch to two
 * different nodes.  Put one of the nodes into 'transmit' mode by connecting
 * with the serial monitor and sending a 'T'.  The ping node sends the current
 * time to the pong node, which responds by sending the value back.  The ping
 * node can then see how long the whole cycle took.
 */

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "examples/GettingStarted/printf.h"

// Set up nRF24L01 radio on SPI bus plus pins 9 & 10

RF24 radio(9,10);

// Radio pipe addresses for the 2 nodes to communicate.
//2(3D + N) + X
const uint64_t pipes[2] = { 0x000000003CLL, 0x000000003DLL };

//byte 1
//8bits of position

//byte 2
//1bit north wall
//1bit south wall
//1bit east wall
//1bit west wall
//1bit robot
//2bits treasure shape //cic,tri,sq,none
//1bit treasure color //red,blue

typedef union {
  unsigned c: 8;
  struct {
    unsigned t_color: 1;
    unsigned t_shape: 2;
    unsigned robot: 1;
    unsigned west: 1;
    unsigned east: 1;
    unsigned south: 1;
    unsigned north: 1;
  };
} square_data_t;

square_data_t maze_data[9][9];
unsigned char explored[11];
square_data_t c_square;

typedef struct {
  uint8_t x;
  uint8_t y;
  int8_t dir;
} robot_self_t;

robot_self_t rSelf;

char int_pos(char x, char y) {
  return x*9 + y;
}

char x_pos(char pos) {
  return pos / 9;
}

char y_pos(char pos) {
  return pos % 9;
}

bool check_explored(char x, char y) {
  unsigned char pos = int_pos(x,y);
  if (pos > 81)
    return true;
  return explored[pos/8] & (1<<(pos%8));
}


if(dir == 0) y--;
    if(dir == 1) x++;
    if(dir == 2) y++;
    if(dir == 3) x--;

void inc_pos_dir(robot_self_t* rt){
  rt->dir %= 4;
  switch(rt->dir){
    case 0:
      rt->y--;
      break;
    case 1:
      rt->x++;
      break;
    case 2:
      rt->y++;
      break;
    default:
      rt->x--;
      break;
  }
}

robot_self_t rTester;
bool n_path;
unsigned char test_path[82];
unsigned char path_depth;

bool ids_search(void) {
  rTester.x = rSelf.x;
  rTester.y = rSelf.y;
  rTester.dir = rSelf.dir;
  char n = 0;
  n_path = true;
  while (ids_recursive(n)){
    if (n > 82 || n_path)
      return false;
    n_path = true;
  }
}

bool np_checker(robot_self_t* rt, char turn_r) {
  turn_r += rt->dir;
  switch (turn_r%4) {
    case 0:
      return !(rt->north);
    case 1:
      return !(rt->east);
    case 2:
      return !(rt->south);
    default:
      return !(rt->west);
  }
}


//TODO: Have function update path depth and test_path[]
bool ids_recursive(robot_self_t* rt, char r_depth) {
  if (!check_explored(rTester.x, rTester.y))
    return true;
  if (r_depth < 1){
    n_path = false;
    return false;
  }
  //terminating condition
  char o_x = rt->x;
  char o_y = rt->y;
  char o_dir = rt->dir;
  int d;
  for (d=0; d<4; d++){
    if (d == 2) d++;
    if (np_checker(rt,d)){
      //move tester
      inc_pos_dir(rt);
      if(ids_recursive(rt,r_depth - 1))
        return true;
      //search in that dir didn't work so try next dir
      rt->x = o_x;
      rt->y = o_y;
      rt->dir = o_dir;
    }
  }
  //none of the searches paths give a good node so failed.
  return false;
}

/////////////////////////////////
// Radio Stuff
/////////////////////////////////
void setup_radio(void) {
  radio.begin();

  // optionally, increase the delay between retries & # of retries
  radio.setRetries(15,15);
  radio.setAutoAck(true);
  // set the channel
  radio.setChannel(0x50);
  // set the power
  // RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_MED=-6dBM, and RF24_PA_HIGH=0dBm.
  radio.setPALevel(RF24_PA_MIN);
  //RF24_250KBPS for 250kbs, RF24_1MBPS for 1Mbps, or RF24_2MBPS for 2Mbps
  radio.setDataRate(RF24_250KBPS);

  // optionally, reduce the payload size.  seems to
  // improve reliability
  radio.setPayloadSize(2);

  // Open pipes to other nodes for communication
  // This simple sketch opens two pipes for these two nodes to communicate
  // back and forth.
  // Open 'our' pipe for writing
  // Open the 'other' pipe for reading, in position #1 (we can have up to 5 pipes open for reading)
  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1,pipes[1]);

  // Start listening
  radio.startListening();

  // Dump the configuration of the rf unit for debugging
  radio.printDetails();
}

bool transmit_radio(char arr[], int n) {
  radio.stopListening();
  Serial.write("sending");
  Serial.write("\n");
  Serial.print((uint8_t) arr[0]);
  Serial.write("  ");
  Serial.print((uint8_t) arr[1]);
  Serial.write("\n");
  bool ok = radio.write(arr, n);

  if (ok)
    printf("ok...");
  else {
    printf("failed.\n\r");
    return false;
  }

  // Now, continue listening
  radio.startListening();

  // Wait here until we get a response, or timeout (250ms)
  unsigned long started_waiting_at = millis();
  bool timeout = false;
  while ( ! radio.available() && ! timeout )
    if (millis() - started_waiting_at > 200 )
      timeout = true;

  // Describe the results
  if ( timeout )
  {
    printf("Failed, response timed out.\n\r");
    return false;
  }
  else
  {
    // Grab the response, compare, and send to debugging spew
    unsigned char ret_val[16];
    radio.read(ret_val, n);

    // Spew it
    Serial.write("recieved");
    Serial.write("\n");
    Serial.print((uint8_t) ret_val[0]);
    Serial.write("  ");
    Serial.print((uint8_t) ret_val[1]);
    Serial.write("\n");
    return true;
  }
}

void ds_tester(void) {
  c_square.north = 1;
  c_square.west = 1;
}

void setup(void)
{
  //
  // Print preamble
  //

  ds_tester();

  Serial.begin(57600);
  printf_begin();
  printf("\n\rRF24/examples/GettingStarted/\n\r");
  printf("ROLE: %s\n\r",role_friendly_name[role]);
  printf("*** PRESS 'T' to begin transmitting to the other node\n\r");

  setup_radio();
}

void loop(void)
{
  char node[16];
  node[0] = 1;
  node[1] = c_square.c;//0x50;
  while(!transmit_radio(node,2)){
    delay(1000);
  }
  delay(1000);
  node[0] = 10;
  node[1] = 0x40;
  while(!transmit_radio(node,2)){
    delay(1000);
  }
  delay(1000);
  node[0] = 19;
  node[1] = 0x60;
  while(!transmit_radio(node,2)){
    delay(1000);
  }
  delay(1000);
  node[0] = 18;
  node[1] = 0xA0;
  while(!transmit_radio(node,2)){
    delay(1000);
  }
  delay(1000);
  node[0] = 9;
  node[1] = 0x80;
  while(!transmit_radio(node,2)){
    delay(1000);
  }
  delay(1000);
  node[0] = 0;
  node[1] = c_square.c;//0x90;
  while(!transmit_radio(node,2)){
    delay(1000);
  }
  delay(1000);
}
