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

bool backLeftW = true;
bool backRightW = true;
bool frontLeftW = true;
bool frontRightW = true;

int thresh  = 300;

char* directions = "lrrrrlll";
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

void stop()
{
  runServo(90,90);
}




void adjustRight()
{
  runServo(95, 90); // left , right 
}

void adjustLeft()
{
  runServo(90, 85); // left, right 
}

void turnRight()
{
  runServo(92, 92); // left , right 
}


void turnLeft()
{
  runServo(88, 88); // left, right 
}



// Go straight for runTime msecs
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
}

void goStraightOneBlock()
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

void turnIntersection(char direction)
{
   if(backRightW && backLeftW)
  {
    if(direction == 'l')
    {
      turnLeft();
      delay(1100);
      while(!(frontRightW && frontLeftW))
      {
        readStatus();
      }
      goStraight();
      delay(300);
    } else 
    {
      turnRight();
      delay(1100);
      while(!(frontRightW && frontLeftW))
      {
        readStatus();
      }
       goStraight();
       delay(300); 
    }
    count+=1;
    if(count == 8) count = 0;
  }
  
}

void loop() {
 
  goStraightOneBlock();
  
  delay(1);




  turnIntersection(directions[count]);

 

  
}







