#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

// Hardware configuration
// Set up nRF24L01 radio on SPI bus plus pins 9 & 10
RF24 radio(9,10);

// Topology
// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = { 0x000000003CLL, 0x000000003DLL };

// Role management
// The various roles supported by this sketch
typedef enum { role_ping_out = 1, role_pong_back } role_e;

// The debug-friendly names of those roles
const char* role_friendly_name[] = { "invalid", "Ping out", "Pong back"};

//Payload from radio ----------------------
byte payload[3];
boolean newData = false;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

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

  //pong back pipes
  radio.openWritingPipe(pipes[1]);
  radio.openReadingPipe(1,pipes[0]);
  
  radio.startListening();

  //default original
  //Serial.println("0,0,north=true,west=true");
  
}

byte * readRadio(){
    if ( radio.available() )
    {
      // Dump the payloads until we've gotten everything
      bool done = false;
      while (!done)
      {
        // Fetch the payload, and see if this was the last one.
        done = radio.read( &payload, sizeof(unsigned short) );

        //Serial.print((uint8_t) payload[0]);
        //Serial.write("  ");
        //Serial.print((uint8_t) payload[1]);
        // Spew it
        //printf("Got payload %lu...",got_time);

        // Delay just a little bit to let the other unit
        // make the transition to receiver
        delay(20);

      }

      // First, stop listening so we can talk
      radio.stopListening();

      // Send the final one back.
      radio.write( &payload, sizeof(unsigned short) );

      // Now, resume listening so we catch the next packets.
      radio.startListening();
      newData = true;
    }
    //return payload;
}

void loop() {
  // Run through a 2x3 maze
//  Serial.println("reset");
//  delay(1000);
//  Serial.println("0,0,north=true,west=true");
//  delay(1000);
//  Serial.println("0,1,north=true");
//  delay(1000);
//  Serial.println("0,2,north=true,east=true");
//  delay(1000);
//  Serial.println("1,2,south=true,east=true");
//  delay(5000);
//  Serial.println("1,1,south=true");
//  delay(1000);
//  Serial.println("1,0,south=true,west=true");
//  delay(1000);
  readRadio();
  if(newData){
    uint8_t coord = payload[0];
    uint8_t info = payload[1];
    //Serial.println(coord);
    //Serial.println(info);
    String x = String(coord%9);
    
    String y = String(coord/9);
    Serial.print(y+",");
    Serial.print(x);

    int north = (info&0x80);
    if(north) Serial.print(",north=true");
    else Serial.print(",north=false");
    
    int south = (info&0x40);
    if(south) Serial.print(",south=true");
    else Serial.print(",south=false");
    
    int east = (info&0x20);
    if(east) Serial.print(",east=true");
    else Serial.print(",east=false");
    
    int west = (info&0x10);
    if(west) Serial.print(",west=true");
    else Serial.print(",west=false");
    
    Serial.print("\n");
    newData = false;
  }
}

