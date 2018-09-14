#include <Servo.h>

Servo servo0;  // create servo object for the left servo
Servo servo1;  // create servo object for the right servo

int backLeft = A2; // pin for back left sensor
int frontRight = A0; // pin for front right sensor
int frontLeft = A1; // pin for front left sensor
int backRight = A3; // pin for back right sensor

bool backLeftW = true;
bool backRightW = true;
bool frontLeftW = true;
bool frontRightW = true;

int thresh  = 300; // threshold value for analog line follower input to read true

char* directions = "lrrrrlll"; // command vector
int count = 0;

// Helper function that runs servos with leftSpeed, rightSpeed for delayTime msecs
void runServo(int leftSpeed, int rightSpeed)
{
  servo0.write(leftSpeed); 
  servo1.write(rightSpeed);
}

// Turn 90 degrees to the right
void readStatus()
{
  backLeftW = analogRead(backLeft) < thresh;
  backRightW = analogRead(backRight) < thresh;
  frontLeftW = analogRead(frontLeft) < thresh;
  frontRightW = analogRead(frontRight) < thresh;
}

// stop the servo
void stopServo()
{
  runServo(90,90);
}

// adjust the robot to veer right
void adjustRight()
{
  runServo(95, 90); // left , right 
}

// adjust the robot to veer left
void adjustLeft()
{
  runServo(90, 85); // left, right 
}

//  turn towards the right
void turnRight()
{
  runServo(92, 92); // left , right 
}

// turn towards the left
void turnLeft()
{
  runServo(88, 88); // left, right 
}


// Go straight
void goStraight()
{
   runServo(92, 88);
}

void setup() 
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN,LOW);
  servo0.attach(5,1300,1700);  // attaches the servo on pin 5 to the servo object
  servo1.attach(6,1300,1700);  // attaches the servo on pin 6 to the servo object
  Serial.begin(9600);
}

// follows line, adjusts autonomously
void lineFollower()
{
   readStatus();
  
  if(frontRightW && frontLeftW)
  {
    goStraight();
    
  } else if(!frontLeftW && frontRightW)
  {
    adjustRight();
    
  } else if(frontLeftW && !frontRightW)
  {
    adjustLeft();
  }
}

// turn when it encounters an intersection
void turnIntersection(char direction)
{
   if(backRightW && backLeftW) // intersection
  {
    if(direction == 'l') // command array tells it to turn left
    {
      turnLeft(); // set motors to turn left
      delay(1100);
      while(!(frontRightW && frontLeftW)) // keep turning left until both front sensors read white
      {
        readStatus();
      }
      goStraight(); // move forward
      delay(300);
    } 
    else // command array tells it to turn right
    {
      turnRight(); // set motors to turn right
      delay(1100);
      while(!(frontRightW && frontLeftW)) // keep turning left until both front sensors read white
      {
        readStatus();
      }
       goStraight(); // move forward
       delay(300); 
    }
    count+=1; // increment through character array
    if(count == 8) count = 0; // reset figure 8 loop
  }
}

void loop() {
 
  followLine();
  delay(1);

  turnIntersection(directions[count]);
  
}
