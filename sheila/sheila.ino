#include <Servo.h>

Servo servo0;  // create servo object for the left servo
Servo servo1;  // create servo object for the right servo
int lineSensors[5] = {0, 0, 0, 0, 0};
int pos;
int proportional;
int derivative;
int last_proportional;
int integral = 0;
int k1 = 20;
int k2 = 1;
int k3 = 1;

// Helper function that runs servos with leftSpeed, rightSpeed for delayTime msecs
void runServo(int leftSpeed, int rightSpeed)
{
  servo0.write(90+leftSpeed); 
  servo1.write(90-rightSpeed);
}

void readLineSensors()
{
  lineSensors[0] = !(digitalRead(9));
  lineSensors[1] = !(digitalRead(10));
  lineSensors[2] = !(digitalRead(11));
  lineSensors[3] = !(digitalRead(12));
  lineSensors[4] = !(digitalRead(13));
  
  if(lineSensors[0]) pos = 2;
  else if(lineSensors[1]) pos = 1;
  else if(lineSensors[2]) pos = 0;
  else if(lineSensors[3]) pos = -1;
  else if(lineSensors[4]) pos = -2;

  Serial.println(pos);
  int power_difference = pos*k1;
  if(power_difference > 0)
      runServo(90-power_difference, 90);
  else
      runServo(90, 90-power_difference);
}

void PID()
{
  proportional = pos;
  derivative = proportional - last_proportional;
  integral = integral + proportional;
  last_proportional = proportional;

  float power_difference = proportional * k1 + integral * k2 + derivative * k3;

  // Compute the motor settings. The motor's accepts values between 0 and 180. 

  if(power_difference > 90)
     power_difference = 90;
  if(power_difference < -90)
     power_difference = -90;

  // set the motors to the correct speed
  if(power_difference < 0)
      runServo(90-power_difference, 90);
  else
      runServo(90, 90-power_difference);
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

void setup() 
{
  servo0.attach(3,1300,1700);  // attaches the servo on pin 5 to the servo object
  servo1.attach(5,1300,1700);  // attaches the servo on pin 6 to the servo object
  pinMode(9, INPUT);
  pinMode(10, INPUT);
  pinMode(11, INPUT);
  pinMode(12, INPUT);
  pinMode(13, INPUT);
  Serial.begin(9600);
}

void loop() 
{
  readLineSensors();
  //PID();
  delay(100);
}
