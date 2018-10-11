#define LOG_OUT 1 // use the log output function
#define FFT_N 256 // set to 256 point fft

#include <FFT.h> // include the library
#include <Servo.h>



Servo servo0;  // create servo object for the left servo
Servo servo1;  // create servo object for the right servo

int line[5] = {0, 0, 0, 0, 0};
int error = 0;
int Kp = 19;
int originalSpeed = 50;
int motorSpeedL = 0;
int motorSpeedR = 0;
int counter = 1001;
int turn = 0;

char* directions = "lrrrrlll";
int count = 0;
int result;

void setup() {
  Serial.begin(115200);
  servo0.attach(3,1300,1700);  // attaches the servo on pin 5 to the servo object
  servo1.attach(5,1300,1700);  // attaches the servo on pin 6 to the servo object
  TIMSK0 = 0; // turn off timer0 for lower jitter
  ADCSRA = 0xe5; // set the adc to free running mode
  ADMUX = 0x40; // use adc0
  DIDR0 = 0x01; // turn off the digital input for adc0
  pinMode(9, INPUT);
  pinMode(10, INPUT);
  pinMode(11, INPUT);
  pinMode(12, INPUT);
  pinMode(13, INPUT);
}

void loop() {
 
  if (!checkIntersection())
  {
    PIDControl();
    counter = (counter > 150000) ? counter: counter + 1;
    if (counter == 250) count = (count+1) % 8;
  } else 
  {
    if(directions[count] == 'l')
    {
      turnLeftSweep();
    } else 
    {
      turnRightSweep();
    }
    counter = 0;
  }
  
  result = readIR();
  Serial.println("exit IR");
  if(result == 1)
  {
    Serial.println("Stop Found IR");
    make180turn2();
  } else 
  {
    Serial.println("NO IR");
    goStraight();
    //PIDControl();
  }
  result=0;
}


void make180turn()
{
  turnRightSweep();
 
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
    for (int i = 0 ; i < 512 ; i += 2) { // save 256 samples
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
    //sei();
    if(fft_log_out[39] > 120 || fft_log_out[40] > 120 || fft_log_out[41] > 120){
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

void turnRightSweep()
{
  runServo(60,-20);
  mydelay(500);
  IRmeasurements();
  runServo(60,60);
  mydelay(50);
  while(line[0] + line[1] + line[2] + line[3] + line[4] < 2) {
    runServo(60,-20);
    IRmeasurements();
  }
}

void mydelay(int count)
{
  while(count > 0)
  {
    count-= 1;
  }
}

void turnLeftSweep() {
  runServo(-20,60);
  mydelay(450);
  IRmeasurements();
  runServo(60,60);
  mydelay(50);
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

int checkIntersection() 
{
  line[0] = !(digitalRead(9));
  line[1] = !(digitalRead(10));
  line[2] = !(digitalRead(11));
  line[3] = !(digitalRead(12));
  line[4] = !(digitalRead(13));
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
