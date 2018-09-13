#include <Servo.h>

Servo servo0;  // create servo object for the left servo
Servo servo1;  // create servo object for the right servo

int backLeft = A2; // pin for back left sensor
int frontRight = A0; // pin for front right sensor
int frontLeft = A1; // pin for front left sensor
int backRight = A3; // pin for back right sensor

int backLeftInput = 0;
int frontRightInput = 0;
int frontLeftInput = 0;
int backRightInput = 0;

int thresh  = 700;

char* directions = "lrrrrlll";
int count = 0;
// Helper function that runs servos with leftSpeed, rightSpeed for delayTime msecs
void runServo(int leftSpeed, int rightSpeed)
{
  servo0.write(leftSpeed); 
  servo1.write(rightSpeed);
}

// Turn 90 degrees to the right
void turnRight()
{
  runServo(160, 160);
}

void stop()
{
  servo0.detach();
  servo1.detach();
}

void adjustRight()
{
  runServo(95, 90); // left , right 
}

void adjustLeft()
{
  runServo(90, 85); // left, right 
}



// Go straight for runTime msecs
void goStraight()
{
   runServo(95, 85);
}

void setup() 
{
  servo0.attach(5,1300,1700);  // attaches the servo on pin 5 to the servo object
  servo1.attach(6,1300,1700);  // attaches the servo on pin 6 to the servo object
}

void loop() {
  backLeftInput = analogRead(backLeft);
  frontRightInput = analogRead(frontRight);
  frontLeftInput = analogRead(frontLeft);
  backRightInput = analogRead(backRight);
  
  if(frontRightInput < thresh && frontLeftInput < thresh)
  {
    goStraight();
    
  } else if(frontLeftInput > thresh && frontRightInput < thresh)
  {
    adjustRight();
    
  } else if(frontLeftInput < thresh && frontRightInput > thresh)
  {
    adjustLeft();
  }
  delay(2);

  if(backRightInput < thresh || backLeftInput < thresh)
  {
    if(directions[count] == 'l')
    {
      adjustLeft();
      delay(100);
      while(!(frontRightInput < thresh && frontLeftInput < thresh))
      {
      adjustLeft();
      }
    } else 
    {
      adjustRight();
      delay(100);
      while(!(frontRightInput < thresh && frontLeftInput < thresh))
      {
      adjustRight();
      } 
    }
  }
  count+=1;
  if(count == 8) count = 0;

  
}
