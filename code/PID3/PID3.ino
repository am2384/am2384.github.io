#define LOG_OUT 1 // use the log output function
#define FFT_N 256 // set to 256 point fft

#include <FFT.h> // include the library
#include <Servo.h>

Servo servo0;  // create servo object for the left servo
Servo servo1;  // create servo object for the right servo

int line[5] = {0, 0, 0, 0, 0};
int error = 0;
int Kp = 12;
int originalSpeed = 50;
int motorSpeedL = 0;
int motorSpeedR = 0;
int counter = 1001;
int turn = 0;
char* directions = "lrrrrlll";
int count = 0;
int result;
int sensorPinRight = A1;
int sensorValueRight;
int sensorPinFront = A3;
int sensorValueFront;
int start = 0;
int dir = 0;
int x = 0;
int y = 0;

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

int self_x = 0, self_y = 0;

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
}

void loop() {
//  if(start == 0)
//  {
//    while(audio() == 0)
//    {
//      Serial.println("No tone");
//    }
//    start = 1;
//  }
  
  if(readIR()==1)
  {
    Serial.println("IR Hat Detected");
    make180turn();
  } 
  else
  {
    //Serial.println("");
  }

  if (!checkIntersection()) // we are not at an intersection
  {
    PIDControl();
  } 
  else // we are at an intersection
  {
    Serial.println("Intersection");
    if(rightSensor() == 0)
    { 
    turnRightSweep();
      if(dir == 0) y++;
      if(dir == 1) x++;
      if(dir == 2) y--;
      if(dir == 3) x--;
      if(dir == 0) c_square.east = 0;
      else if (dir == 1) c_square.south = 0;
      else if (dir == 2) c_square.west = 0;
      else c_square.north = 0;
      dir++;      
      if(dir == 4) dir = 0;
    } 
    else if(frontSensor() == 1) 
    {
      turnLeftSweep();
      if(dir == 0) y++;
      if(dir == 1) x++;
      if(dir == 2) y--;
      if(dir == 3) x--;
      if(dir == 0){
        c_square.east = 1;
        c_square.north = 1;
      }
      else if (dir == 1){
        c_square.south = 1;
        c_square.east = 1;
      }
      else if (dir == 2){
        c_square.west = 1;
        c_square.south = 1;
      }
      else{
        c_square.north = 1;
        c_square.west = 1;
      }
      dir--;
      if(dir < 0) dir = 3;
    }
    else
    {
      PIDControl();
      if(dir == 0) y++;
      if(dir == 1) x++;
      if(dir == 2) y--;
      if(dir == 3) x--;
      if(dir == 0) c_square.east = 1;
      else if (dir == 1) c_square.south = 1;
      else if (dir == 2) c_square.west = 1;
      else c_square.north = 1;
    }
    
    Serial.print(" North:");
    Serial.print(c_square.north);
    Serial.print(" East:");
    Serial.print(c_square.east);
    Serial.print(" South:");
    Serial.print(c_square.south);
    Serial.print(" West:");
    Serial.print(c_square.west);
    Serial.print(" Dir:");
    Serial.print(dir);
    Serial.print(" X:");
    Serial.print(x);
    Serial.print(" Y:");
    Serial.print(y);
    Serial.print(" Dir:");
    Serial.println(dir);
  }
}

int frontSensor()
{
    sensorValueFront = analogRead(sensorPinFront);
  if(sensorValueFront<300) return 0;
  else return 1;
}

int rightSensor()
{
  sensorValueRight = analogRead(sensorPinRight);
  if(sensorValueRight<300) return 0;
  else return 1;
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
    //Serial.println(fft_log_out[39]);
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
