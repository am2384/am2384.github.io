

#include <FFT.h> // include the library
#include <Servo.h>

Servo servo0;  // create servo object for the left servo
Servo servo1;  // create servo object for the right servo

int line[5] = {0, 0, 0, 0, 0};
int error = 0;
int Kp = 19;
int Ki = 2;
int Kd = 2;
int dT = 1;
int originalSpeed = 50;
int motorSpeedL = 0;
int motorSpeedR = 0;
int errorSum = 0;
int errorDiff = 0;
int prev_error = 0;
int counter = 1001;

char* directions = "lrrrrlll";
int count = 0;



void setup() {
  //Serial.begin(9600);
  servo0.attach(3,1300,1700);  // attaches the servo on pin 5 to the servo object
  servo1.attach(5,1300,1700);  // attaches the servo on pin 6 to the servo object
  pinMode(9, INPUT);
  pinMode(10, INPUT);
  pinMode(11, INPUT);
  pinMode(12, INPUT);
  pinMode(13, INPUT);
}

void loop() {
  if (!checkIntersection()){
    PIDControl();
    counter = counter > 150000 ? counter: counter + 1;
    if (counter == 250)
      count = (count+1) % 8;
  }
  else {
    if(directions[count] == 'l')
    {
      turnLeftSweep();
    } else 
    {
      turnRightSweep();
    }
    counter = 0;
  }
}

void PIDControl()
{
  error = IRmeasurements();
  errorSum += error * dT;
  errorDiff = (error - prev_error)/dT;
  prev_error = IRmeasurements(); 
  
  motorSpeedL = -(Kp*error) + originalSpeed; // 
  motorSpeedR = +(Kp*error) + originalSpeed;
 
  runServo(motorSpeedL, motorSpeedR);    // remember error is negative if it turns left 
 
}

void turnRightSweep() {
  runServo(60,-20);
  delay(500);
  IRmeasurements();
  runServo(60,60);
  delay(50);
  while(line[0] + line[1] + line[2] + line[3] + line[4] < 2) {
    runServo(60,-20);
    IRmeasurements();
  }
}

void turnLeftSweep() {
  runServo(-20,60);
  delay(450);
  IRmeasurements();
  runServo(60,60);
  delay(50);
  while(line[0] + line[1] + line[2] + line[3] + line[4] < 2) {
    runServo(-20,60);
    IRmeasurements();
  }
}

void runServo(int leftSpeed, int rightSpeed)
{
  servo0.write(90+leftSpeed);  
  servo1.write(90-rightSpeed); 
}

int checkIntersection() {
  IRmeasurements();
  return line[0] && line[1] && line[2] && line[3] && line[4];
}

int IRmeasurements()
{
  line[0] = !(digitalRead(9));
  line[1] = !(digitalRead(10));
  line[2] = !(digitalRead(11));
  line[3] = !(digitalRead(12));
  line[4] = !(digitalRead(13));
  
  if(line[2] == 1 && line[0] == 0 && line[1] == 0 && line[3] == 0 && line[4] == 0)  ///////////////// 2 only straight 
  {
    error = 0;
  } else if (line[2] == 1 && line[1] == 1 && line[0] == 0  && line[3] == 0 && line[4] == 0) ///////////// 1 and 2  = 1xleft
  {
    error = +1;
  } else if (line[2] == 1 && line[3] == 1 && line[0] == 0  && line[1] == 0 && line[4] == 0) ////////////  2 and 3 =  1xright 
  {
    error = -1;
  }else if(line[1] == 1 && line[2] == 0 &&  line[0] == 0  && line[3] == 0 && line[4] == 0) ///////////// 1 only =  2xleft 
  {
    error = +2;
  } else if(line[3] == 1 && line[1] == 0 && line[0] == 0  && line[3] == 0 && line[4] == 0) /////////// 3 only =  2xright 
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

