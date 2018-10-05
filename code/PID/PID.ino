/*
fft_adc_serial.pde
guest openmusiclabs.com 7.7.14
example sketch for testing the fft library.
it takes in data on ADC0 (Analog0) and processes them
with the fft. the data is sent out over the serial
port at 115.2kb.
*/

#define LOG_OUT 1 // use the log output function
#define FFT_N 256 // set to 256 point fft

#include <FFT.h> // include the library
#include <Servo.h>

Servo servo0;  // create servo object for the left servo
Servo servo1;  // create servo object for the right servo
int line[5] = {0, 0, 0, 0, 0};
int pos;
int proportional;
int derivative;
int last_proportional;
int integral = 0;
int k1 = 20;
int k2 = 1;
int k3 = 1;

// Adeel
int error = 0;
int Kp = 2;
int Ki = 2;
int Kd = 2;
int originalSpeed = 45;
int motorSpeedL = 0;
int motorSpeedR = 0;
int errorSum = 0;
int errorDiff = 0;
int prev_error = 0;



void setup() {
//  Serial.begin(115200); // use the serial port
//  TIMSK0 = 0; // turn off timer0 for lower jitter
//  ADCSRA = 0xe5; // set the adc to free running mode
//  ADMUX = 0x40; // use adc0
//  DIDR0 = 0x01; // turn off the digital input for adc0

  // from Sheila
  servo0.attach(3,1300,1700);  // attaches the servo on pin 5 to the servo object
  servo1.attach(5,1300,1700);  // attaches the servo on pin 6 to the servo object
  pinMode(9, INPUT);
  pinMode(10, INPUT);
  pinMode(11, INPUT);
  pinMode(12, INPUT);
  pinMode(13, INPUT);
}

void loop() {
//  readLineSensors();
//  int result = readIR();
//  if(result == 1){
//    Serial.println("IR Hat!");
//  }
//  delay(100);


}

int readIR(){
  cli();  // UDRE interrupt slows this way down on arduino1.0
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
    sei();
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
  errorSum += error * dT;
  errorDiff = (error - prev_error)/dT;
  prev_error = IRmeasurements(); 
  motorSpeedL = +(Kp*error + Ki*errorSum + Kd*errorDiff) + originalSpeed; // orignial Speed =  45;
  motorSpeedR = -(Kp*error + Ki*errorSum + Kd*errorDiff ) + originalSpeed;
  
  runServo(motorSpeedL - originalSpeed, motorSpeedR - originalSpeed);    // remember error is negative if it turns left 
  
}

void runServo(int leftSpeed, int rightSpeed)
{
  servo0.write(90+leftSpeed);  
  servo1.write(90-rightSpeed); 
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
    error = -5;
  } else if (line[2] == 1 && line[3] == 1 && line[0] == 0  && line[1] == 0 && line[4] == 0) ////////////  2 and 3 =  1xright 
  {
    error = +5;
  }else if(line[1] == 1 && line[2] == 0 &&  line[0] == 0  && line[3] == 0 && line[4] == 0) ///////////// 1 only =  2xleft 
  {
    error = -7;
  } else if(line[3] == 1 && line[1] == 0 && line[0] == 0  && line[3] == 0 && line[4] == 0) /////////// 3 only =  2xright 
  {
    error = + 7;
  } else if(line[0] == 1 && line[1] == 1 && line[2] == 0  && line[3] == 0 && line[4] == 0) //////////     0 and 1 =  3x left  
  {
    error = -9;
  } else if (line[3] == 1 && line[4] == 1 && line[0] == 0  && line[1] == 0 && line[2] == 0) ////////   3 and 4 =  3x right 
  {
    error = +9;
  } else if (line[0] == 1 && line[1] == 0 && line[2] == 0  && line[3] == 0 && line[4] == 0) /////    0 only = 4x left 
  {
    error = -11;
  } else if(line[4] == 1 && line[0] == 0 && line[1] == 0  && line[2] == 0 && line[3] == 0) ////// 4 only = 4x right 
  {
    error = + 11;
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






