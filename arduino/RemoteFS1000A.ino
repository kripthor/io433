#include <EEPROM.h>

#define LED LED_BUILTIN

//PINS FOR THE FS1000A
#define OUTVIN 9
#define OUTDATA A0
#define OUTGND 10

//PWM SIGNAL CONSTANTS
#define REPEATSIGNAL 8
#define SIGNAL_PERIOD 1200
#define SIGNAL_TIME SIGNAL_PERIOD/3
#define SIGNAL_PREAMBLE 2300
#define SIGNAL_INTERVAL 24400

// Define some time critical functions to avoid function calls.
// If needed, this can be optimized further. But works well as is for above timings.

#define SENDWAIT(B,T) digitalWrite(OUTDATA, B); delayMicroseconds(T);

// There is a 66% (active low) duty cycle for '1' and 33% for '0'
// Rougly translates to: 001 -> encodes '1'  /  011 -> encodes '0'
#define SENDBIT(B,T) SENDWAIT(0,T); SENDWAIT(!B,T); SENDWAIT(1,T); 

// ddddddd(gate id?) 7f(button pressed:7f/bf/df/ef) 98fb('rolling key')
// You need to change to your own gate ID!
//                     d        d        d        d        d         d        d        7        f       9       8        f        b
byte signal433[] = {1,1,0,1, 1,1,0,1, 1,1,0,1, 1,1,0,1, 1,1,0,1, 1,1,0,1, 1,1,0,1, 0,1,1,1, 1,1,1,1, 1,0,0,1, 1,0,0,0, 1,1,1,1, 1,0,1,1};
byte signalLength = sizeof(signal433)/sizeof(signal433[0]);


void blinky() {
  int i = 0;
  while (i++ <= 3) {
     digitalWrite(LED, HIGH);
     delay(75);
     digitalWrite(LED, LOW);
     delay(75);
   }
}

void randomizeKey() {
   // 'randomly' change 'rolling key'
  randomSeed(EEPROM.read(0) ^ EEPROM.read(1));
  byte r0 = (byte)random(255);
  byte r1 = (byte)random(255);
  EEPROM.write(0,r0);
  EEPROM.write(1,r1);
  for (int j=0;j<8;j++) {
    signal433[36+j] = ((r0>>j)&1);
    signal433[44+j] = ((r1>>j)&1);
  };
}


void setup() {
  Serial.begin(115200);
  pinMode(OUTVIN, OUTPUT);
  pinMode(OUTGND, OUTPUT);
  pinMode(OUTDATA, OUTPUT);
  pinMode(LED, OUTPUT);
  digitalWrite(OUTVIN,HIGH);
  digitalWrite(OUTGND,LOW);
  digitalWrite(OUTDATA,LOW);
  randomizeKey();
}

void loop() {
  int i,t;
  for (t=0; t<REPEATSIGNAL;t++){   
    
      //SEND PREAMBLE
      SENDWAIT(1,SIGNAL_PREAMBLE);

      //SEND BITS
      for (i = 0; i < signalLength; i++){
        SENDBIT(signal433[i], SIGNAL_TIME);
      }
      
      //WAIT SOME INTERVAL
      SENDWAIT(0,SIGNAL_INTERVAL); 
  }
  
  // A PAUSE AND A BLINK
  blinky();
  delay(2000);
}