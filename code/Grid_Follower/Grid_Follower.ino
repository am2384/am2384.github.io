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

void loop() {
  backLeftInput = (analogRead(backLeft)*10 + backLeftInput*22)>>5;
  frontRightInput = (analogRead(frontRight)*10 + frontRightInput*22)>>5;
  frontLeftInput = (analogRead(frontLeft)*10 + frontLeftInput*22)>>5 ;
  backRightInput = (analogRead(backRight)*10 + backRightInput*22)>>5;
  
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
  delay(1);

/*
if(backRightInput < thresh && backLeftInput < thresh)
  {
       
      turnLeft();
      delay(800);
      digitalWrite(LED_BUILTIN, HIGH);
      while(!(frontRightInput < thresh && frontLeftInput < thresh))
      {
         backLeftInput = (analogRead(backLeft)*10 + backLeftInput*22)>>5;
         frontRightInput = (analogRead(frontRight)*10 + frontRightInput*22)>>5;
         frontLeftInput = (analogRead(frontLeft)*10 + frontLeftInput*22)>>5 ;
         backRightInput = (analogRead(backRight)*10 + backRightInput*22)>>5;
      }
      digitalWrite(LED_BUILTIN, LOW); 
      goStraight();
      delay(300);
  }
  
*/




  if(backRightInput < thresh && backLeftInput < thresh)
  {
    if(directions[count] == 'l')
    {
      turnLeft();
      delay(800);
      while(!(frontRightInput < thresh && frontLeftInput < thresh))
      {
        backLeftInput = analogRead(backLeft);
        frontRightInput = analogRead(frontRight);
        frontLeftInput = analogRead(frontLeft);
        backRightInput = analogRead(backRight);
      }
      goStraight();
      delay(300);
    } else 
    {
      turnRight();
      delay(800);
      while(!(frontRightInput < thresh && frontLeftInput < thresh))
      {
        backLeftInput = analogRead(backLeft);
        frontRightInput = analogRead(frontRight);
        frontLeftInput = analogRead(frontLeft);
        backRightInput = analogRead(backRight);
      }
       goStraight();
       delay(300); 
    }
  }
  count+=1;
  if(count == 8) count = 0;

  
}
