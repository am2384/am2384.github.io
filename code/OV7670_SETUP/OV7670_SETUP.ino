#include <Wire.h>

#define OV7670_I2C_ADDRESS 0x42 /*TODO: write this in hex (eg. 0xAB) */


///////// Main Program //////////////
void setup() {
  Wire.begin();
  Serial.begin(9600);

  // TODO: READ KEY REGISTERS
  read_key_registers();
  
  delay(100);
  /*
  // TODO: WRITE KEY REGISTERS
  Serial.println(OV7670_write_register(0x12,0x80));   // COM 7 bit 7 - set to 1 to reset
  Serial.println(OV7670_write_register(0x0C,0x08));   // COM 3 bit 3 - set to 1 to enable scaling
  Serial.println(OV7670_write_register(0x11,0x40));   // CLK RC bit 6 - set to 1 for external clock
  OV7670_write_register(0x12,0x06);//0x04 for no color bar   // COM 7 bit 2- RGB format (bit 2- set to 1, bit 0- set to 0)
  OV7670_write_register(0x40,0xD0); // COM 15- bit 5:4 -> 01
  //OV7670_write_register(,);  // MVFP bits 5 (1 = mirror) and bit 4 (1 = vflip)
  //OV7670_write_register(,);   // COM 9 bit 6:4- automatic gain ceiling
  */
  
  read_key_registers();
  set_color_matrix();
}

void loop(){
 }

///////// Function Definition //////////////
void read_key_registers(){
  /*TODO: DEFINE THIS FUNCTION*/
  Serial.println("BEGIN");
  Serial.println(read_register_value(0x12), HEX);
  Serial.println(read_register_value(0x0C), HEX);
  Serial.println(read_register_value(0x11), HEX);
  Serial.println(read_register_value(0x12), HEX);
  Serial.println(read_register_value(0x40), HEX);
  Serial.println("END");
}

byte read_register_value(int register_address){
  byte data = 0;
  Wire.beginTransmission(OV7670_I2C_ADDRESS);
  Wire.write(register_address);
  Wire.endTransmission();
  Wire.requestFrom(OV7670_I2C_ADDRESS,1);
  while(Wire.available()<1);
  data = Wire.read();
  return data;
}

String OV7670_write(int start, const byte *pData, int size){
    int n,error;
    Serial.println("writing");
    Wire.beginTransmission(OV7670_I2C_ADDRESS);
    n = Wire.write(start);
    if(n != 1){
      Serial.println("I2C ERROR WRITING START ADDRESS");
      return "I2C ERROR WRITING START ADDRESS";   
    }
    n = Wire.write(pData, size);
    if(n != size){
      Serial.println("I2C ERROR WRITING DATA");
      return "I2C ERROR WRITING DATA";
    }
    error = Wire.endTransmission(true);
    if(error != 0){
      return String(error);
    }
    Serial.println("no errors");
    return "no errors :)";
 }

String OV7670_write_register(int reg_address, byte data){
  return OV7670_write(reg_address, &data, 1);
 }

void set_color_matrix(){
    OV7670_write_register(0x4f, 0x80);
    OV7670_write_register(0x50, 0x80);
    OV7670_write_register(0x51, 0x00);
    OV7670_write_register(0x52, 0x22);
    OV7670_write_register(0x53, 0x5e);
    OV7670_write_register(0x54, 0x80);
    OV7670_write_register(0x56, 0x40);
    OV7670_write_register(0x58, 0x9e);
    OV7670_write_register(0x59, 0x88);
    OV7670_write_register(0x5a, 0x88);
    OV7670_write_register(0x5b, 0x44);
    OV7670_write_register(0x5c, 0x67);
    OV7670_write_register(0x5d, 0x49);
    OV7670_write_register(0x5e, 0x0e);
    OV7670_write_register(0x69, 0x00);
    OV7670_write_register(0x6a, 0x40);
    OV7670_write_register(0x6b, 0x0a);
    OV7670_write_register(0x6c, 0x0a);
    OV7670_write_register(0x6d, 0x55);
    OV7670_write_register(0x6e, 0x11);
    OV7670_write_register(0x6f, 0x9f);
    OV7670_write_register(0xb0, 0x84);
}
