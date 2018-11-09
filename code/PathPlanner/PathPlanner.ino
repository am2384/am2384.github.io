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

robot_self_t self;

int int_pos(int x, int y) {
  return x*9 + y;
}

int x_pos(int pos) {
  return pos / 9;
}

int y_pos(int pos) {
  return pos % 9;
}

bool check_explored(char x, char y) {
  unsigned char pos = int_pos(x,y);
  if (pos > 81)
    return true;
  return explored[pos/8] & (1<<(pos%8));
}

void set_explored(char x, char y, bool e) {
  unsigned char pos = int_pos(x,y);
  if (pos > 81)
    return;
  if (e)
    explored[pos/8] |= (1<<(pos%8));
}

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
  rTester.x = self.x;
  rTester.y = self.y;
  rTester.dir = self.dir;
  char n = 1;
  n_path = true;
  path_depth = 0;
  while (!ids_recursive( &rTester, n)){
    if (n > 82 || n_path)
      return false;
    n_path = true;
    n++;
    //Serial.println(">>>>>>>>>>>>>>>>>>>> New Search");
  }
  return true;
}

int np_checker(robot_self_t* rt, char turn_r) {
  turn_r += rt->dir;
  switch (turn_r%4) {
    case 0:
      return !(maze_data[rt->x][rt->y].north);
    case 1:
      return !(maze_data[rt->x][rt->y].east);
    case 2:
      return !(maze_data[rt->x][rt->y].south);
    default:
      return !(maze_data[rt->x][rt->y].west);
  }
}

int check_old_path(robot_self_t* rt) {
  int x;
  int pos = int_pos(rt->x, rt->y);
  if (pos == int_pos(self.x, self.y))
    return 1;
  for (x = 0; x<path_depth-1; x++){
    if (test_path[x] == pos)
      return 1;
  }
  return 0;
}

//TODO: Have function update path depth and test_path[]
bool ids_recursive(robot_self_t* rt, int r_depth) {
  /*Serial.println("recursive!");
  Serial.println(rt->x);
  Serial.println(rt->y);*/
  if (!check_explored(rt->x, rt->y))
    return true;
  if (r_depth < 1){
    n_path = false;
    return false;
  }
  //terminating condition
  int o_x = rt->x;
  int o_y = rt->y;
  int o_dir = rt->dir;
  int d;
  for (d=0; d<4; d++){
    if (np_checker(rt,d)){
      //move tester
      rt->dir = (rt->dir + d) % 4;
      inc_pos_dir(rt);
      test_path[path_depth] = int_pos(rt->x, rt->y);
      path_depth++;
      if (check_old_path(rt)){
        path_depth--;
        //search in that dir didn't work so try next dir
        rt->x = o_x;
        rt->y = o_y;
        rt->dir = o_dir;
        continue;
      }
      if(ids_recursive(rt,r_depth - 1))
        return true;
      path_depth--;
      //search in that dir didn't work so try next dir
      rt->x = o_x;
      rt->y = o_y;
      rt->dir = o_dir;
    } /*else {
      Serial.println("wall");
    }*/
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
  Serial.begin(9600);
  self.dir = 0; //0=N,1=E,2=S,3=W
  self.x = 0;
  self.y = 1;

  set_explored(0, 0, 1);
  maze_data[0][0].north = 1;
  maze_data[0][0].west = 1;

  set_explored(1, 0, 1);
  maze_data[1][0].north = 1;
  maze_data[1][0].east = 1;

  set_explored(1, 1, 1);
  maze_data[1][1].east = 1;

  set_explored(1, 2, 1);
  maze_data[1][2].south = 1;
  maze_data[1][2].east = 1;

  set_explored(0, 2, 1);
  maze_data[0][2].north = 1;
  maze_data[0][2].west = 1;

  set_explored(0, 3, 1);
  maze_data[0][3].west = 1;

  set_explored(1, 3, 1);
  maze_data[1][3].north = 1;
  maze_data[1][3].south = 1;

  set_explored(2, 3, 1);
  maze_data[2][3].north = 1;

  set_explored(3, 3, 1);
  maze_data[3][3].east = 1;

  set_explored(3, 2, 1);
  maze_data[3][2].east = 1;

  set_explored(3, 1, 1);
  maze_data[3][1].west = 1;
  maze_data[3][1].east = 1;

  set_explored(3, 0, 1);
  maze_data[3][0].north = 1;
  maze_data[3][0].west = 1;
  maze_data[3][0].east = 1;

  set_explored(2, 2, 1);
  maze_data[2][2].west = 1;
  maze_data[2][2].south = 1;

  set_explored(2, 1, 1);
  maze_data[2][1].west = 1;
  maze_data[2][1].east = 1;

  set_explored(2, 0, 1);
  maze_data[2][0].north = 1;
  maze_data[2][0].west = 1;
  maze_data[2][0].east = 1;

  set_explored(3, 4, 1);
  maze_data[3][4].south = 1;
  maze_data[3][4].east = 1;

  set_explored(2, 4, 1);
  maze_data[2][4].south = 1;
  maze_data[2][4].west = 1;

  set_explored(0, 4, 1);
  maze_data[0][4].south = 1;
  maze_data[0][4].west = 1;

  set_explored(1, 4, 1);
  maze_data[1][4].north = 1;
  maze_data[1][4].south = 1;
  maze_data[1][4].east = 1;

  set_explored(0, 1, 1);
  maze_data[0][1].south = 1;
  maze_data[0][1].west = 1;

  Serial.println(ids_search());
  Serial.println(path_depth);
  Serial.println("pos");
  int n;
  for (n = 0; n < path_depth; n++){
    Serial.println(x_pos(test_path[n]));
    Serial.println(y_pos(test_path[n]));
    Serial.println("");
  }
}

void loop(void)
{
  
}
