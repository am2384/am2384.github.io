#define LOG_OUT 1 // use the log output function
#define FFT_N 256 // set to 256 point fft

#include <FFT.h> // include the library
#include <Servo.h>
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

Servo servo0;  // create servo object for the left servo
Servo servo1;  // create servo object for the right servo

int line[5] = {0, 0, 0, 0, 0};
int walls[3] = {0, 0, 0};
int error = 0;
int Kp = 12;
int originalSpeed = 50;
int motorSpeedL = 0;
int motorSpeedR = 0;
int turn = 0;
int result;
int sensorPinRight = A1;
int sensorValueRight;
int sensorPinFront = A3;
int sensorValueFront;
int sensorPinLeft = A4;
int sensorValueLeft;
int start = 0;
char node[2];

// radio
RF24 radio(9,10);
const uint64_t pipes[2] = { 0x000000003CLL, 0x000000003DLL };

typedef struct {
  uint8_t x;
  uint8_t y;
  int8_t dir;
} robot_self_t;

robot_self_t self;

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
square_data_t c_square;

void setup() {
  //Serial.begin(115200);
  Serial.begin(9600);
  servo0.attach(3,1300, 1700);  // attaches the servo on pin 5 to the servo object
  servo1.attach(5,1300, 1700);  // attaches the servo on pin 6 to the servo object
  pinMode(6, INPUT);
  pinMode(8, INPUT);
  pinMode(2, INPUT);
  pinMode(4, INPUT);
  pinMode(7, INPUT);
  self.dir = 2; //0=N,1=E,2=S,3=W
  self.x = 2;
  self.y = 0;
  setup_radio();
}

void loop() {
  /*if(start == 0)
  {
    while(audio() == 0)
    {
      Serial.println("No tone");
    }
    start = 1;
  }*/
  
//  if(readIR()==1)
//  {
//    Serial.println("IR Hat Detected");
//    make180turn();
//  } 
//  else
//  {
//    Serial.println("");
//  }

  //leftSensor();

  if (!checkIntersection()) // we are not at an intersection
  {
    PIDControl();
  } 
  else // we are at an intersection
  {
    Serial.println("Intersection");
    stopServos();
    inc_pos(&self);
    update_walls(&self);
    node[0] = self.y*9 + self.x;
    node[1] = maze_data[self.x][self.y].c;
    //while(!transmit_radio(node,2)){}

    // A bunch of debug serial prints
    Serial.print(" North:");
    Serial.print(maze_data[self.x][self.y].north);
    Serial.print(" East:");
    Serial.print(maze_data[self.x][self.y].east);
    Serial.print(" South:");
    Serial.print(maze_data[self.x][self.y].south);
    Serial.print(" West:");
    Serial.print(maze_data[self.x][self.y].west);
    Serial.print(" X:");
    Serial.print(self.x);
    Serial.print(" Y:");
    Serial.print(self.y);
    Serial.print(" Dir:");
    Serial.println(self.dir);

    if(rightSensor() == 0)
    { 
      turnRightSweep();
      rec_right_turn(&self);
    } 
    else if(frontSensor() == 1) 
    {
      turnLeftSweep();
      rec_left_turn(&self);
    }
    else
    {
      PIDControl();
      int s_time = millis();
      while (s_time + 100 > millis())
        PIDControl();
    }
  }
}

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
  //radio.printDetails();
}

bool transmit_radio(char arr[], int n) {
  radio.stopListening();
  //  Serial.write("sending");
  //  Serial.write("\n");
  //  Serial.print((uint8_t) arr[0]);
  //  Serial.write("  ");
  //  Serial.print((uint8_t) arr[1]);
  //  Serial.write("\n");
  bool ok = radio.write(arr, n);

  if (ok){}
  //    printf("ok...");
  else {
  //    printf("failed.\n\r");
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
  //    printf("Failed, response timed out.\n\r");
    return false;
  }
  else
  {
    // Grab the response, compare, and send to debugging spew
    unsigned char ret_val[16];
    radio.read(ret_val, n);

    // Spew it
  //    Serial.write("recieved");
  //    Serial.write("\n");
  //    Serial.print((uint8_t) ret_val[0]);
  //    Serial.write("  ");
  //    Serial.print((uint8_t) ret_val[1]);
  //    Serial.write("\n");
    return true;
  }
}

int frontSensor()
{
  sensorValueFront = analogRead(sensorPinFront);
  //Serial.println(sensorValueFront);
  if(sensorValueFront<300) return 0;
  else return 1;
}

int rightSensor()
{
  sensorValueRight = analogRead(sensorPinRight);
  //Serial.println(sensorValueRight);
  if(sensorValueRight<200) return 0;
  else return 1;
}

int leftSensor()
{
  sensorValueLeft = analogRead(sensorPinLeft);
  //Serial.println(sensorValueLeft);
  if (sensorValueLeft<200) return 0;
  return 1;
}

void update_walls(robot_self_t* t)
{
  walls[0] = leftSensor();
  walls[1] = frontSensor();
  walls[2] = rightSensor();
  switch(t->dir){
    case 0:
      maze_data[t->x][t->y].west = walls[0];
      maze_data[t->x][t->y].north = walls[1];
      maze_data[t->x][t->y].east = walls[2];
      break;
    case 1:
      maze_data[t->x][t->y].north = walls[0];
      maze_data[t->x][t->y].east = walls[1];
      maze_data[t->x][t->y].south = walls[2];
      break;
    case 2:
      maze_data[t->x][t->y].east = walls[0];
      maze_data[t->x][t->y].south = walls[1];
      maze_data[t->x][t->y].west = walls[2];
      break;
    default:
      maze_data[t->x][t->y].south = walls[0];
      maze_data[t->x][t->y].west = walls[1];
      maze_data[t->x][t->y].north = walls[2];
      break;
  }
}

void make180turn()
{
    runServo(50, -30);
    delay(2000);
    runServo(10, 10);
    delay(400);
}

void make180turn2()
{
   while(turn<1250)
   {
     runServo(60,-20);
     Serial.println("Turning Turning");
     turn++;
   }
   turn=0;
}

int readIR(){
    //cli();  // UDRE interrupt slows this way down on arduino1.0
    byte adcsra_temp = ADCSRA;
    byte adcmux_temp = ADMUX;
    byte didr0_temp = DIDR0;
    TIMSK0 = 0; // turn off timer0 for lower jitter
    ADCSRA = 0xe5; // set the adc to free running mode
    ADMUX = 0x40; // use adc0 //required for fft!
    DIDR0 = 0x01; // turn off the digital input for adc0
    for (int i = 0 ; i < 512 ; i += 2) 
    { // save 256 samples
      while(!(ADCSRA & 0x10)); // wait for adc to be ready
      ADCSRA = 0xf5; // restart adc
      byte m = ADCL; // fetch adc data
      byte j = ADCH;
      int k = (j << 8) | m; // form into an int
      k -= 0x0200; // form into a signed int
      k <<= 6; // form into a 16b signed int
      fft_input[i] = k; // put real data into even bins
      fft_input[i+1] = 0; // set odd bins to 0
    }
    fft_window(); // window the data for better frequency response
    fft_reorder(); // reorder the data before doing the fft
    fft_run(); // process the data in the fft
    fft_mag_log(); // take the output of the fft
    TIMSK0 = 1; // turn on timer0 for lower jitter
    ADCSRA = adcsra_temp; // set the adc to free running mode
    ADMUX = adcmux_temp; // use adc0 //required for fft!
    DIDR0 = didr0_temp; // turn off the digital input for adc0
    //sei();
    Serial.print(fft_log_out[40]);
    Serial.print(" ");
    Serial.print(fft_log_out[41]);
    Serial.print(" ");
    Serial.println(fft_log_out[42]);
    if(fft_log_out[39] > 120 || fft_log_out[40] > 120 || fft_log_out[41] > 120)
    {
      return 1;
    }
    else{
      return 0;
    } 
}

void PIDControl()
{
  error = IRmeasurements();
  motorSpeedL = -(Kp*error) + originalSpeed; 
  motorSpeedR = +(Kp*error) + originalSpeed;

  runServo(motorSpeedL, motorSpeedR);    // remember error is negative if it turns left 
}

int audio()
{
      cli();
    for (int i = 0 ; i < 512 ; i += 2) {
      fft_input[i] = analogRead(A2); // <-- NOTE THIS LINE
      fft_input[i+1] = 0;
    }
    fft_window();
    fft_reorder();
    fft_run();
    fft_mag_log();
    sei();
  //    Serial.println("start");
  //    for (byte i = 0 ; i < FFT_N/2 ; i++) {
  //      Serial.println(fft_log_out[i]);
  //    }

    if(fft_log_out[19] > 65 || fft_log_out[20] > 65 || fft_log_out[21] > 65)
    {
      return 1;
    }
    else{
      return 0;
    }
  
}

void turnRightSweep()
{
  runServo(50, -30);
  delay(900);
  runServo(10, 10);
  delay(400);
}

void mydelay(int count)
{
  while(count > 0)
  {
    count-= 1;
  }
}

void turnLeftSweep() {
  runServo(-30, 50);
  delay(900);
  runServo(10, 10);
  delay(400);
}

void runServo(int leftSpeed, int rightSpeed)
{
  if(leftSpeed>90) leftSpeed = 90;
  if(rightSpeed>90) rightSpeed = 90;
  servo0.write(90+leftSpeed);  
  servo1.write(90-rightSpeed); 
}

bool checkIntersection() 
{
  line[0] = !(digitalRead(6));
  line[1] = !(digitalRead(8));
  line[2] = !(digitalRead(2));
  line[3] = !(digitalRead(4));
  line[4] = !(digitalRead(7));
  /*Serial.print(line[0]);
  Serial.print(line[1]);
  Serial.print(line[2]);
  Serial.print(line[3]);
  Serial.print(line[4]);
  Serial.println("");*/
  return (line[0] && line[1] && line[2] && line[3] && line[4]);
}

int IRmeasurements()
{
  line[0] = !(digitalRead(6));
  line[1] = !(digitalRead(8));
  line[2] = !(digitalRead(2));
  line[3] = !(digitalRead(4));
  line[4] = !(digitalRead(7));
  //    0   1   2   3   4 
  if(line[2] == 1 && line[0] == 0 && line[1] == 0 && line[3] == 0 && line[4] == 0)  ///////////////// 2 only straight 
  {
    error = 0;
  } else if (line[1] == 1 && line[2] == 1 && line[0] == 0  && line[3] == 0 && line[4] == 0) ///////////// 1 and 2  = 1xleft
  {
    error = +1;
  } else if (line[2] == 1 && line[3] == 1 && line[0] == 0  && line[1] == 0 && line[4] == 0) ////////////  2 and 3 =  1xright 
  {
    error = -1;
  }else if(line[1] == 1 && line[0] == 0 &&  line[2] == 0  && line[3] == 0 && line[4] == 0) ///////////// 1 only =  2xleft 
  {
    error = +2;
  } else if(line[3] == 1 && line[0] == 0 && line[1] == 0  && line[2] == 0 && line[4] == 0) /////////// 3 only =  2xright 
  {
    error = - 2;
  } else if(line[0] == 1 && line[1] == 1 && line[2] == 0  && line[3] == 0 && line[4] == 0) //////////     0 and 1 =  3x left  
  {
    error = +3;
  } else if (line[3] == 1 && line[4] == 1 && line[0] == 0  && line[1] == 0 && line[2] == 0) ////////   3 and 4 =  3x right 
  {
    error = -3;
  } else if (line[0] == 1 && line[1] == 0 && line[2] == 0  && line[3] == 0 && line[4] == 0) /////    0 only = 4x left 
  {
    error = +4;
  } else if(line[4] == 1 && line[0] == 0 && line[1] == 0  && line[2] == 0 && line[3] == 0) ////// 4 only = 4x right 
  {
    error = - 4;
  } 
  return error;
}

void stopServos()
{
  runServo(0, 0);
}

// Go straight
void goStraight()
{
   runServo(90, 90);
}

void inc_pos(robot_self_t* t)
{
  if(t->dir == 0) t->y--;
  else if(t->dir == 1) t->x++;
  else if(t->dir == 2) t->y++;
  else if(t->dir == 3) t->x--;
}

void rec_left_turn(robot_self_t *t)
{
  t->dir = (t->dir-1);
  if(t->dir == -1)
    t->dir = 3;
}

void rec_right_turn(robot_self_t *t)
{
  t->dir = (t->dir+1) % 4;
}
