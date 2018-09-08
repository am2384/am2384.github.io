#include <Servo.h>

Servo servo0;  // create servo object for the left servo
Servo servo1;  // create servo object for the right servo

int backLeft = A2; // back left line0
int frontRight = A0; // front right  1
int frontLeft = A1; // front left  2
int backRight = A3; // back right  3

int backLeftInput = 0;
int frontRightInput = 0;
int frontLeftInput = 0;
int backRightInput = 0;

int thresh  = 700;
// Helper function that runs servos with leftSpeed, rightSpeed for delayTime msecs
void runServo(int leftSpeed, int rightSpeed, int delayTime)
{
  servo0.write(leftSpeed); 
  servo1.write(rightSpeed);
  //delay(delayTime);
}

// Turn 90 degrees to the right
void turnRight()
{
  runServo(160, 160, 556);
}

void adjustRight(int val)
{
  runServo(95, 90, val); // left , right 
}

void adjustLeft(int val)
{
  runServo(90, 85, val); // left, right 
}



// Go straight for runTime msecs
void goStraight(int runTime)
{
   runServo(95, 85, runTime);
}

void setup() {
  servo0.attach(5,1300,1700);  // attaches the servo on pin 5 to the servo object
  servo1.attach(6,1300,1700);  // attaches the servo on pin 6 to the servo object
  
  Serial.begin(9600);
  
  
}
// ECE 3400 folder
void loop() {
  backLeftInput = analogRead(backLeft);
  frontRightInput = analogRead(frontRight);
  frontLeftInput = analogRead(frontLeft);
  backRightInput = analogRead(backRight);
  
  if(frontRightInput < thresh && frontLeftInput < thresh)
  {
    goStraight(500);
  } else if(frontLeftInput > thresh && frontRightInput < thresh)
  {
    adjustRight(10);
 
  } else if(frontLeftInput < thresh && frontRightInput > thresh)
  {
    adjustLeft(10);
  }
  delay(5);
}

