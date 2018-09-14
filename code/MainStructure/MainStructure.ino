#include <Servo.h>

Servo servo0;  // create servo object for the left servo
Servo servo1;  // create servo object for the right servo

int backLeft = A2; // pin for back left sensor
int frontRight = A0; // pin for front right sensor
int frontLeft = A1; // pin for front left sensor
int backRight = A3; // pin for back right sensor

bool backLeftWTE = false;
bool frontRightWTE = false;
bool frontLeftWTE = false;
bool backRightWTE = false;

bool backLeftBLK = true;
bool frontRightBLK = true;
bool frontLeftBLK = true;
bool backRightBLK = true;

// Helper function that runs servos with leftSpeed, rightSpeed for delayTime msecs
void runServo(int leftSpeed, int rightSpeed)
{
  servo0.write(leftSpeed); 
  servo1.write(rightSpeed);
}

void adjustRight()
{
  runServo(95, 90); // left , right 
}

void adjustLeft()
{
  runServo(90, 85); // left, right 
}

// Turn 90 degrees to the right
void turnLeft()
{
  readStatus();
  runServo(88, 88);
  delay(100);
  while(frontRightBLK && frontLeftBLK)
  {
    delay(100);
    readStatus();
    Serial.println("In Left Loop");
  }
  Serial.println("Exit Left Loop");
  goStraight();
  delay(300);
  runServo(90, 90);
}

// Turn 90 degrees to the right
void turnRight()
{
  runServo(92, 92);
}

void goStraightOneBlock()
{
  Serial.println("Go Straight One Block");
  readStatus();
  while(backRightBLK && backLeftBLK)
  {
    readStatus();
    if(frontRightWTE && frontLeftWTE)
    {
      goStraight();
    } 
    else if(frontLeftBLK && frontRightWTE)
    {
      adjustRight();
    } 
    else if(frontLeftWTE && frontRightBLK)
    {
      adjustLeft();
    }
  }
  runServo(90, 90);
}

// Go straight
void goStraight()
{
   runServo(95, 85);
}

void readStatus()
{
  int thresh  = 300;
  backLeftWTE = (analogRead(backLeft)<thresh); // white
  frontRightWTE = (analogRead(frontRight)<thresh);
  frontLeftWTE = (analogRead(frontLeft)<thresh);
  backRightWTE = (analogRead(backRight)<thresh);

  backLeftBLK = !backLeftWTE;
  frontRightBLK = !frontRightWTE;
  frontLeftBLK = !frontLeftWTE;
  backRightBLK = !backRightWTE;

//  Serial.print(backLeftWTE);
//  Serial.print(frontLeftWTE);
//  Serial.print(frontRightWTE);
//  Serial.println(backLeftWTE);
}

void setup() 
{
  servo0.attach(5,1300,1700);  // attaches the servo on pin 5 to the servo object
  servo1.attach(6,1300,1700);  // attaches the servo on pin 6 to the servo object
  Serial.begin(9600);
}

void loop() 
{
  goStraightOneBlock();
  turnLeft();
}
