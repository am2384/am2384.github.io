#include <Servo.h>

Servo servo0;  // create servo object for the left servo
Servo servo1;  // create servo object for the right servo

int backLeft = A2; // pin for back left sensor
int frontRight = A0; // pin for front right sensor
int frontLeft = A1; // pin for front left sensor
int backRight = A3; // pin for back right sensor

bool backLeftWTE = false; // true if back left sensor on white
bool frontRightWTE = false; // true if front right sensor on white
bool frontLeftWTE = false; // true if front left sensor on white
bool backRightWTE = false; // true if back right sensor on white

bool backLeftBLK = true; // true if back left sensor on black
bool frontRightBLK = true; // true if back right sensor on black
bool frontLeftBLK = true; // true if back right sensor on black
bool backRightBLK = true; // true if back right sensor on black

// Helper function that runs servos with leftSpeed, rightSpeed
void runServo(int leftSpeed, int rightSpeed)
{
  servo0.write(leftSpeed); 
  servo1.write(rightSpeed);
}

//turns right at slow speed
void adjustRight()
{
  runServo(95, 90); // left , right 
}

//turns left at slow speed
void adjustLeft()
{
  runServo(90, 85); // left, right 
}

// Turn 90 degrees to the left
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
  readStatus();
  runServo(92, 92);
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

// Follow straight line until an intersection is encountered
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

// Read status of four line sensors
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

// Attach servos and begin serial monitor
void setup() 
{
  servo0.attach(5,1300,1700);  // attaches the servo on pin 5 to the servo object
  servo1.attach(6,1300,1700);  // attaches the servo on pin 6 to the servo object
  Serial.begin(9600);
}

// Main function
void loop() 
{
  goStraightOneBlock();
  turnLeft();
}
