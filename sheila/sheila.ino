#include <Servo.h>

#include <FFT.h> // include the library

#define LOG_OUT 1 // use the log output function
#define FFT_N 256 // set to 256 point fft


Servo servo0;  // create servo object for the left servo
Servo servo1;  // create servo object for the right servo
int lineSensors[5] = {0, 0, 0, 0, 0};
int pos;
int proportional;
int derivative;
int last_proportional;
int integral = 0;
int k1 = 20;
int k2 = 1;
int k3 = 1;

// Helper function that runs servos with leftSpeed, rightSpeed for delayTime msecs
void runServo(int leftSpeed, int rightSpeed)
{
  servo0.write(90+leftSpeed); 
  servo1.write(90-rightSpeed);
}

void readLineSensors()
{
  lineSensors[0] = !(digitalRead(9));
  lineSensors[1] = !(digitalRead(10));
  lineSensors[2] = !(digitalRead(11));
  lineSensors[3] = !(digitalRead(12));
  lineSensors[4] = !(digitalRead(13));
  
  if(lineSensors[0]) pos = 2;
  else if(lineSensors[1]) pos = 1;
  else if(lineSensors[2]) pos = 0;
  else if(lineSensors[3]) pos = -1;
  else if(lineSensors[4]) pos = -2;

  Serial.println(pos);
  int power_difference = pos*k1;
  if(power_difference > 0)
      runServo(90-power_difference, 90);
  else
      runServo(90, 90-power_difference);
}

void PID()
{
  proportional = pos;
  derivative = proportional - last_proportional;
  integral = integral + proportional;
  last_proportional = proportional;

  float power_difference = proportional * k1 + integral * k2 + derivative * k3;

  // Compute the motor settings. The motor's accepts values between 0 and 180. 

  if(power_difference > 90)
     power_difference = 90;
  if(power_difference < -90)
     power_difference = -90;

  // set the motors to the correct speed
  if(power_difference < 0)
      runServo(90-power_difference, 90);
  else
      runServo(90, 90-power_difference);
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

void setup() 
{
  servo0.attach(3,1300,1700);  // attaches the servo on pin 5 to the servo object
  servo1.attach(5,1300,1700);  // attaches the servo on pin 6 to the servo object
  pinMode(9, INPUT);
  pinMode(10, INPUT);
  pinMode(11, INPUT);
  pinMode(12, INPUT);
  pinMode(13, INPUT);
  //from FFT
  Serial.begin(115200); // use the serial port
  TIMSK0 = 0; // turn off timer0 for lower jitter
  ADCSRA = 0xe5; // set the adc to free running mode
  ADMUX = 0x40; // use adc0
  DIDR0 = 0x01; // turn off the digital input for adc0
}

void loop() 
{
  readLineSensors();
  int result = readIR();
  if(result == 1){
    Serial.println("IR Hat!");
  }
  //PID();
  delay(100);
}
