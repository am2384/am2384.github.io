#define LOG_OUT 1 // use the log output function
#define FFT_N 256 // set to 256 point fft
#define IR_SAMPLES 60
#define IR_ROBOT_SAMPLES 5
#define IR_THRESH 3


#include <FFT.h> // include the library
#include <Servo.h>
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

Servo servo0;  // create servo object for the left servo
Servo servo1;  // create servo object for the right servo

int line[5] = {0, 0, 0, 0, 0};
int walls[3] = {0, 0, 0};
int error = 0;
int Kp = 31;
int originalSpeed = 90;//origin 50
int motorSpeedL = 0;
int motorSpeedR = 0;
int turn = 0;
int result;
int sensorPinRight = A1;
int sensorValueRight;
int sensorPinFront = A3;
int sensorValueFront;
int sensorPinLeft = A4;
int sensorValueLeft;
char node[2];
int IRcounter=0;
int temp_Arry[IR_SAMPLES];
bool map_changed = false;

// radio
RF24 radio(9, 10);
const uint64_t pipes[2] = { 0x000000003CLL, 0x000000003DLL };

typedef struct {
  uint8_t x;
  uint8_t y;
  int8_t dir;
} robot_self_t;

robot_self_t self;

typedef union {
  unsigned c: 8;
  struct {
    unsigned t_color: 1;
    unsigned t_shape: 2;
    unsigned robot: 1;
    unsigned west: 1;
    unsigned east: 1;
    unsigned south: 1;
    unsigned north: 1;
  };
} square_data_t;

square_data_t maze_data[9][9];
unsigned char explored[11];
unsigned int c_path_depth = 0;
//square_data_t c_square;

//Path Planning Variables
robot_self_t rTester;
bool n_path;
unsigned char test_path[82];
unsigned char turns[82];
unsigned int path_depth = 0;

////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////

void setup() {
  //Serial.begin(115200);
  Serial.begin(9600);
  pinMode(6, INPUT);
  pinMode(8, INPUT);
  pinMode(2, INPUT);
  pinMode(4, INPUT);
  pinMode(7, INPUT);
  self.dir = 2; //0=N,1=E,2=S,3=W
  self.x = 0;
  self.y = 0;
  //TODO: Setup First Square Here
  set_explored(0, 0, 1);
  maze_data[0][0].north = 1;
  maze_data[0][0].west = 1;

  setup_radio();

  while (audio() == 0)
  {
    //Serial.println(IR_det());
  }
  servo0.attach(3, 1300, 1700); // attaches the servo on pin 5 to the servo object
  servo1.attach(5, 1300, 1700); // attaches the servo on pin 6 to the servo object
  Serial.println("Intersection");
  stopServos();
  update_walls(&self, false);
  set_explored(self.x, self.y, 1);
  node[0] = self.y * 9 + self.x;
  node[1] = maze_data[self.x][self.y].c;
  while(!transmit_radio(node,2)){}

  // A bunch of debug serial prints
  Serial.print(" North:");
  Serial.print(maze_data[self.x][self.y].north);
  Serial.print(" East:");
  Serial.print(maze_data[self.x][self.y].east);
  Serial.print(" South:");
  Serial.print(maze_data[self.x][self.y].south);
  Serial.print(" West:");
  Serial.print(maze_data[self.x][self.y].west);
  Serial.print(" X:");
  Serial.print(self.x);
  Serial.print(" Y:");
  Serial.print(self.y);
  Serial.print(" Dir:");
  Serial.println(self.dir);

  while (!ids_search()){}
  // path planning debug statements
  Serial.println(ids_search());
  Serial.println(path_depth);
  Serial.println("pos");
  int n;
  for (n = 0; n < path_depth; n++) {
    Serial.println(x_pos(test_path[n]));
    Serial.println(y_pos(test_path[n]));
    Serial.println("");
  }
  c_path_depth = 0;
  cmd_intersection();
  
  //update_walls(&self);
  //set_explored(self.x, self.y, 1);
  //node[0] = self.y * 9 + self.x;
  //node[1] = maze_data[self.x][self.y].c;
  //while(!transmit_radio(node,2)){}
}

void loop() {

  //leftSensor();
  
  if (!checkIntersection()) // we are not at an intersection
  {
    PIDControl();
  }
  else // we are at an intersection
  {
    int a;
    for (a=0; a<IR_ROBOT_SAMPLES; a++)
      IRcounter += IR_det();
    if (IRcounter > IR_THRESH)
      IRcounter = 1;
    else
      IRcounter = 0;
    if (IRcounter)
      delay(1000);
    map_changed = false;
    Serial.println("Intersection");
    stopServos();
    inc_pos(&self);
    c_path_depth++;
    map_changed = update_walls(&self, false);
    set_explored(self.x, self.y, 1);
    node[0] = self.y * 9 + self.x;
    node[1] = maze_data[self.x][self.y].c;
    while(!transmit_radio(node,2)){}

    // A bunch of debug serial prints
    Serial.print(" North:");
    Serial.print(maze_data[self.x][self.y].north);
    Serial.print(" East:");
    Serial.print(maze_data[self.x][self.y].east);
    Serial.print(" South:");
    Serial.print(maze_data[self.x][self.y].south);
    Serial.print(" West:");
    Serial.print(maze_data[self.x][self.y].west);
    Serial.print(" X:");
    Serial.print(self.x);
    Serial.print(" Y:");
    Serial.print(self.y);
    Serial.print(" Dir:");
    Serial.println(self.dir);

    if (c_path_depth < path_depth && !map_changed) {
      cmd_intersection();
    } else {
      while (!ids_search()){
        map_changed = update_walls(&self, false);
      }
      
      // path planning debug statements
      Serial.println(ids_search());
      Serial.println(path_depth);
      Serial.println("pos");
      int n;
      for (n = 0; n < path_depth; n++) {
        Serial.println(x_pos(test_path[n]));
        Serial.println(y_pos(test_path[n]));
        Serial.println("");
      }
      c_path_depth = 0;

      if (IRcounter){
        update_walls(&self, true);
        node[0] = self.y * 9 + self.x;
        node[1] = maze_data[self.x][self.y].c;
        while(!transmit_radio(node,2)){}
      }
      
      cmd_intersection();
    }
  }
}

////////////////////////////////////////////////////////
// Helper Functions
////////////////////////////////////////////////////////

void cmd_intersection() {
  if (turns[c_path_depth] == 0) {
    PIDControl();
    unsigned long s_time = millis();;
    while (s_time + 300 > millis()) {
      PIDControl();
    }
  } else if (turns[c_path_depth] == 1) {
    turnRightSweep();
    rec_right_turn(&self);
  } else if (turns[c_path_depth] == 2) {
    make180turn();
    rec_u_turn(&self);
  } else {
    turnLeftSweep();
    rec_left_turn(&self);
  }
}

void setup_radio(void) {
  radio.begin();

  // optionally, increase the delay between retries & # of retries
  radio.setRetries(15, 15);
  radio.setAutoAck(true);
  // set the channel
  radio.setChannel(0x50);
  // set the power
  // RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_MED=-6dBM, and RF24_PA_HIGH=0dBm.
  radio.setPALevel(RF24_PA_MIN);
  //RF24_250KBPS for 250kbs, RF24_1MBPS for 1Mbps, or RF24_2MBPS for 2Mbps
  radio.setDataRate(RF24_250KBPS);

  // optionally, reduce the payload size.  seems to
  // improve reliability
  radio.setPayloadSize(2);

  // Open pipes to other nodes for communication
  // This simple sketch opens two pipes for these two nodes to communicate
  // back and forth.
  // Open 'our' pipe for writing
  // Open the 'other' pipe for reading, in position #1 (we can have up to 5 pipes open for reading)
  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1, pipes[1]);

  // Start listening
  radio.startListening();

  // Dump the configuration of the rf unit for debugging
  //radio.printDetails();
}

bool transmit_radio(char arr[], int n) {
  radio.stopListening();
  //  Serial.write("sending");
  //  Serial.write("\n");
  //  Serial.print((uint8_t) arr[0]);
  //  Serial.write("  ");m m 
  //  Serial.print((uint8_t) arr[1]);
  //  Serial.write("\n");
  bool ok = radio.write(arr, n);

  if (ok) {}
  //    printf("ok...");
  else {
    //    printf("failed.\n\r");
    return false;
  }

  // Now, continue listening
  radio.startListening();

  // Wait here until we get a response, or timeout (250ms)
  unsigned long started_waiting_at = millis();
  bool timeout = false;
  while ( ! radio.available() && ! timeout )
    if (millis() - started_waiting_at > 200 )
      timeout = true;

  // Describe the results
  if ( timeout )
  {
    //    printf("Failed, response timed out.\n\r");
    return false;
  }
  else
  {
    // Grab the response, compare, and send to debugging spew
    unsigned char ret_val[16];
    radio.read(ret_val, n);

    // Spew it
    //    Serial.write("recieved");
    //    Serial.write("\n");
    //    Serial.print((uint8_t) ret_val[0]);
    //    Serial.write("  ");
    //    Serial.print((uint8_t) ret_val[1]);
    //    Serial.write("\n");
    return true;
  }
}

int frontSensor()
{
  long temp = 0;
  int x;
  for (x=0; x < IR_SAMPLES; x++){
    temp += analogRead(sensorPinFront);
  }
  sensorValueFront = (int) (temp/IR_SAMPLES);
  //sensorValueFront = analogRead(sensorPinFront);
  //Serial.println(sensorValueFront);
  if (sensorValueFront < 500) return 0;
  else return 1;
}

int rightSensor()
{
  long temp = 0;
  int x;
  for (x=0; x < IR_SAMPLES; x++){
    temp += analogRead(sensorPinRight);
  }
  sensorValueRight = (int) (temp/IR_SAMPLES);
  //Serial.println(sensorValueRight);
  if (sensorValueRight < 200) return 0;
  else return 1;
}

int leftSensor()
{
  long temp = 0;
  //long var = 0;
  int x;
  for (x=0; x < IR_SAMPLES; x++){
    temp += analogRead(sensorPinLeft);
  }
  sensorValueLeft = (int) (temp/IR_SAMPLES);
  /*for (x=0; x < IR_SAMPLES; x++){
    int t = analogRead(sensorPinLeft);
    var += (sensorValueLeft - t)*(sensorValueLeft - t);
  }*/
  Serial.println(sensorValueLeft);
  //Serial.print(",      ");
  //Serial.println(var);
  if (sensorValueLeft < 200) return 0;
  return 1;
}

bool update_walls(robot_self_t* t, bool ir)
{
  int maze_data_old = maze_data[t->x][t->y].c;
  walls[0] = leftSensor();
  if (ir)
    walls[1] = false;
  else
    walls[1] = frontSensor();
  walls[2] = rightSensor();
  switch (t->dir) {
    case 0:
      maze_data[t->x][t->y].west = walls[0];
      maze_data[t->x][t->y].north = walls[1];
      maze_data[t->x][t->y].east = walls[2];
      break;
    case 1:
      maze_data[t->x][t->y].north = walls[0];
      maze_data[t->x][t->y].east = walls[1];
      maze_data[t->x][t->y].south = walls[2];
      break;
    case 2:
      maze_data[t->x][t->y].east = walls[0];
      maze_data[t->x][t->y].south = walls[1];
      maze_data[t->x][t->y].west = walls[2];
      break;
    default:
      maze_data[t->x][t->y].south = walls[0];
      maze_data[t->x][t->y].west = walls[1];
      maze_data[t->x][t->y].north = walls[2];
      break;
  }
  if (t->x == 0)
    maze_data[t->x][t->y].west = 1;
  if (t->y == 0)
    maze_data[t->x][t->y].north = 1;
  if (t->x == 8)
    maze_data[t->x][t->y].east = 1;
  if (t->y == 8)
    maze_data[t->x][t->y].south = 1;
  if (maze_data_old == maze_data[t->x][t->y].c)
    return false;
  return true;
}

void make180turn()
{
  runServo(50, -30);
  delay(2000);
  runServo(10, 10);
  delay(400);
}

void make180turn2()
{
  while (turn < 1250)
  {
    runServo(60, -20);
    Serial.println("Turning Turning");
    turn++;
  }
  turn = 0;
}

/*
int readIR() {
  //cli();  // UDRE interrupt slows this way down on arduino1.0
  byte adcsra_temp = ADCSRA;
  byte adcmux_temp = ADMUX;
  byte didr0_temp = DIDR0;
  TIMSK0 = 0; // turn off timer0 for lower jitter
  ADCSRA = 0xe5; // set the adc to free running mode
  ADMUX = 0x40; // use adc0 //required for fft!
  DIDR0 = 0x01; // turn off the digital input for adc0
  for (int i = 0 ; i < 512 ; i += 2)
  { // save 256 samples
    while (!(ADCSRA & 0x10)); // wait for adc to be ready
    ADCSRA = 0xf5; // restart adc
    byte m = ADCL; // fetch adc data
    byte j = ADCH;
    int k = (j << 8) | m; // form into an int
    k -= 0x0200; // form into a signed int
    k <<= 6; // form into a 16b signed int
    fft_input[i] = k; // put real data into even bins
    fft_input[i + 1] = 0; // set odd bins to 0
  }
  fft_window(); // window the data for better frequency response
  fft_reorder(); // reorder the data before doing the fft
  fft_run(); // process the data in the fft
  fft_mag_log(); // take the output of the fft
  TIMSK0 = 1; // turn on timer0 for lower jitter
  ADCSRA = adcsra_temp; // set the adc to free running mode
  ADMUX = adcmux_temp; // use adc0 //required for fft!
  DIDR0 = didr0_temp; // turn off the digital input for adc0
  //sei();
  //Serial.print(fft_log_out[40]);
  //Serial.print(" ");
  //Serial.print(fft_log_out[41]);
  //Serial.print(" ");
  //Serial.println(fft_log_out[42]);
  if (fft_log_out[39] > 120 || fft_log_out[40] > 120 || fft_log_out[41] > 120)
  {
    return 1;
  }
  else {
    return 0;
  }
}
*/

void PIDControl()
{
  error = IRmeasurements();
  motorSpeedL = -(Kp * error) + originalSpeed;
  motorSpeedR = +(Kp * error) + originalSpeed;

  runServo(motorSpeedL, motorSpeedR);    // remember error is negative if it turns left
}


int audio()
{
  cli();
  for (int i = 0 ; i < 512 ; i += 2) {
    fft_input[i] = analogRead(A2); // <-- NOTE THIS LINE
    fft_input[i + 1] = 0;
  }
  fft_window();
  fft_reorder();
  fft_run();
  fft_mag_log();
  sei();
//      Serial.println("start");
//      for (byte i = 0 ; i < FFT_N/2 ; i++) {
//        Serial.println(fft_log_out[i]);
//      }

  if (fft_log_out[19] > 65 || fft_log_out[20] > 65 || fft_log_out[21] > 65)
  {
    return 1;
  }
  else {
    return 0;
  }

}



int IR_det()
{
  cli();
  for (int i = 0 ; i < 512 ; i += 2) {
    fft_input[i] = analogRead(A0); // <-- NOTE THIS LINE
    fft_input[i + 1] = 0;
  }
  fft_window();
  fft_reorder();
  fft_run();
  fft_mag_log();
  sei();
//      Serial.println("start");
//      for (byte i = 0 ; i < FFT_N/2 ; i++) {
//        Serial.println(fft_log_out[i]);
//      }

  if (fft_log_out[72] > 50 || fft_log_out[73] > 50 || fft_log_out[74] > 50)
  {
    return 1;
  }
  else {
    return 0;
  }

}



void turnRightSweep()
{
  runServo(50, -30);
  delay(900);
  runServo(10, 10);
  delay(300);
}

void mydelay(int count)
{
  while (count > 0)
  {
    count -= 1;
  }
}

void turnLeftSweep() {
  runServo(-30, 50);//origin(-30,50)
  delay(900);
  runServo(10, 10);
  delay(300);
}

void runServo(int leftSpeed, int rightSpeed)
{
  if (leftSpeed > 90) leftSpeed = 90;
  if (rightSpeed > 90) rightSpeed = 90;
  if (leftSpeed < -90) leftSpeed = -90;
  if (rightSpeed < -90) rightSpeed = -90;
  servo0.write(90+leftSpeed);
  servo1.write(90-rightSpeed);
  //servo0.write(90);
  //servo1.write(90);
}

bool checkIntersection()
{
  line[0] = !(digitalRead(6));
  line[1] = !(digitalRead(8));
  line[2] = !(digitalRead(2));
  line[3] = !(digitalRead(4));
  line[4] = !(digitalRead(7));

    //Serial.print(line[0]);
    //Serial.print(line[1]);
    //Serial.print(line[2]);
    //Serial.print(line[3]);
    //Serial.print(line[4]);
    //Serial.println("");
  if ((line[0] + line[1] + line[2] + line[3] + line[4]) >= 4) return 1;
  return 0;
  //return (line[0] && line[1] && line[2] && line[3] && line[4]);
}

int IRmeasurements()
{
  line[0] = !(digitalRead(6));
  line[1] = !(digitalRead(8));
  line[2] = !(digitalRead(2));
  line[3] = !(digitalRead(4));
  line[4] = !(digitalRead(7));
  //    0   1   2   3   4
  if (line[2] == 1 && line[0] == 0 && line[1] == 0 && line[3] == 0 && line[4] == 0) ///////////////// 2 only straight
  {
    error = 0;
  } else if (line[1] == 1 && line[2] == 1 && line[0] == 0  && line[3] == 0 && line[4] == 0) ///////////// 1 and 2  = 1xleft
  {
    error = +1;
  } else if (line[2] == 1 && line[3] == 1 && line[0] == 0  && line[1] == 0 && line[4] == 0) ////////////  2 and 3 =  1xright
  {
    error = -1;
  } else if (line[1] == 1 && line[0] == 0 &&  line[2] == 0  && line[3] == 0 && line[4] == 0) ///////////// 1 only =  2xleft
  {
    error = +2;
  } else if (line[3] == 1 && line[0] == 0 && line[1] == 0  && line[2] == 0 && line[4] == 0) /////////// 3 only =  2xright
  {
    error = - 2;
  } else if (line[0] == 1 && line[1] == 1 && line[2] == 0  && line[3] == 0 && line[4] == 0) //////////     0 and 1 =  3x left
  {
    error = +3;
  } else if (line[3] == 1 && line[4] == 1 && line[0] == 0  && line[1] == 0 && line[2] == 0) ////////   3 and 4 =  3x right
  {
    error = -3;
  } else if (line[0] == 1 && line[1] == 0 && line[2] == 0  && line[3] == 0 && line[4] == 0) /////    0 only = 4x left
  {
    error = +4;
  } else if (line[4] == 1 && line[0] == 0 && line[1] == 0  && line[2] == 0 && line[3] == 0) ////// 4 only = 4x right
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

void inc_pos(robot_self_t* t)
{
  if (t->dir == 0) t->y--;
  else if (t->dir == 1) t->x++;
  else if (t->dir == 2) t->y++;
  else if (t->dir == 3) t->x--;
}

void rec_left_turn(robot_self_t* t)
{
  t->dir = (t->dir - 1);
  if (t->dir == -1)
    t->dir = 3;
}

void rec_right_turn(robot_self_t* t)
{
  t->dir = (t->dir + 1) % 4;
}

void rec_u_turn(robot_self_t* t)
{
  t->dir = (t->dir + 2) % 4;
}

//////////////////////////////////////
// Path Planner Stuff
//////////////////////////////////////

int int_pos(int x, int y) {
  return x * 9 + y;
}

int x_pos(int pos) {
  return pos / 9;
}

int y_pos(int pos) {
  return pos % 9;
}

bool check_explored(char x, char y) {
  unsigned char pos = int_pos(x, y);
  if (pos > 81)
    return true;
  return explored[pos / 8] & (1 << (pos % 8));
}

void set_explored(char x, char y, bool e) {
  unsigned char pos = int_pos(x, y);
  if (pos > 81)
    return;
  if (e)
    explored[pos / 8] |= (1 << (pos % 8));
}

bool ids_search(void) {
  rTester.x = self.x;
  rTester.y = self.y;
  rTester.dir = self.dir;
  char n = 1;
  n_path = true;
  path_depth = 0;
  while (!ids_recursive( &rTester, n)) {
    if (n > 82 || n_path)
      return false;
    n_path = true;
    n++;
    //Serial.println(">>>>>>>>>>>>>>>>>>>> New Search");
  }
  return true;
}

int np_checker(robot_self_t* rt, char turn_r) {
  turn_r += rt->dir;
  switch (turn_r % 4) {
    case 0:
      return !(maze_data[rt->x][rt->y].north);
    case 1:
      return !(maze_data[rt->x][rt->y].east);
    case 2:
      return !(maze_data[rt->x][rt->y].south);
    default:
      return !(maze_data[rt->x][rt->y].west);
  }
}

int check_old_path(robot_self_t* rt) {
  int x;
  int pos = int_pos(rt->x, rt->y);
  if (pos == int_pos(self.x, self.y))
    return 1;
  for (x = 0; x < path_depth - 1; x++) {
    if (test_path[x] == pos)
      return 1;
  }
  return 0;
}

//TODO: Have function update path depth and test_path[]
bool ids_recursive(robot_self_t* rt, int r_depth) {
  /*Serial.println("recursive!");
    Serial.println(rt->x);
    Serial.println(rt->y);*/
  if (!check_explored(rt->x, rt->y))
    return true;
  if (r_depth < 1) {
    n_path = false;
    return false;
  }
  //terminating condition
  int o_x = rt->x;
  int o_y = rt->y;
  int o_dir = rt->dir;
  int d;
  for (d = 0; d < 4; d++) {
    if (np_checker(rt, d)) {
      //move tester
      rt->dir = (rt->dir + d) % 4;
      inc_pos(rt);
      test_path[path_depth] = int_pos(rt->x, rt->y);
      turns[path_depth] = d;
      path_depth++;
      if (check_old_path(rt)) {
        path_depth--;
        //search in that dir didn't work so try next dir
        rt->x = o_x;
        rt->y = o_y;
        rt->dir = o_dir;
        continue;
      }
      if (ids_recursive(rt, r_depth - 1))
        return true;
      path_depth--;
      //search in that dir didn't work so try next dir
      rt->x = o_x;
      rt->y = o_y;
      rt->dir = o_dir;
    } /*else {
      Serial.println("wall");
    }*/
  }
  //none of the searches paths give a good node so failed.
  return false;
}
