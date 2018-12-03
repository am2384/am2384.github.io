#define LOG_OUT 1 // use the log output function
#define FFT_N 256 // set to 256 point fft

#include <FFT.h> // include the library



void setup() {
   Serial.begin(9600);

}

void loop() {
  Serial.println(audio());

}
// IR = 6000 Hz appears as an alias at 85th bin. 
// Audio = 660 Hz appears as an alias at 20th bin. 
// IR = 18000 Hz appears as an alias at 6th bin.  
// 4000 Hz appears at 116th bin. 
// 4300 Hz appears at 124th bin.  
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
