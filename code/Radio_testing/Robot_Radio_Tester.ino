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

//
// Hardware configuration
//

// Set up nRF24L01 radio on SPI bus plus pins 9 & 10

RF24 radio(9,10);

//
// Topology
//

// Radio pipe addresses for the 2 nodes to communicate.
//2(3D + N) + X
const uint64_t pipes[2] = { 0x000000003CLL, 0x000000003DLL };

//
// Role management
//
// Set up role.  This sketch uses the same software for all the nodes
// in this system.  Doing so greatly simplifies testing.
//

// The various roles supported by this sketch
typedef enum { role_ping_out = 1, role_pong_back } role_e;

// The debug-friendly names of those roles
const char* role_friendly_name[] = { "invalid", "Ping out", "Pong back"};

// The role of the current running sketch
role_e role = role_ping_out;


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
  Serial.write(arr,2);
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
    Serial.write(ret_val, 2);
    Serial.write("\n");
  }
}

void setup(void)
{
  //
  // Print preamble
  //

  Serial.begin(57600);
  printf_begin();
  printf("\n\rRF24/examples/GettingStarted/\n\r");
  printf("ROLE: %s\n\r",role_friendly_name[role]);
  printf("*** PRESS 'T' to begin transmitting to the other node\n\r");

  setup_radio();
}

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

void loop(void)
{
  char node[16];
  node[1] = 0;
  node[2] = 0x98;
  while(~transmit_radio(node,2));
  delay(1000);
  node[1] = 9;
  node[2] = 0x80;
  while(~transmit_radio(node,2));
  delay(1000);
  node[1] = 18;
  node[2] = 0xA0;
  while(~transmit_radio(node,2));
  delay(1000);
  node[1] = 19;
  node[2] = 0x60;
  while(~transmit_radio(node,2));
  delay(5000);
  node[1] = 10;
  node[2] = 0x40;
  while(~transmit_radio(node,2));
  delay(1000);
  node[1] = 1;
  node[2] = 0x50;
  while(~transmit_radio(node,2));
  delay(1000);
}
