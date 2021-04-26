#include <EEPROM.h>

/// PINS FOR THE FS1000A 433mhz emitter
#define OUTVIN 9
#define OUTDATA A0
#define OUTGND 10

#define LED LED_BUILTIN
#define REPEATSIGNAL 8
#define SIGNAL_PERIOD 1200
#define SIGNAL_PREAMBLE 2300
#define SIGNAL_INTERVAL 24400

#define SENDWAIT(B,T) digitalWrite(OUTDATA, B); delayMicroseconds(T);

// ddddddd(gate id?) 7f(button pressed:7f/bf/df/ef) 98fb('rolling key')
// You need to change to your own gate ID!
byte signal433[] = {1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,0,1,1,1,1,1,1,1,1,0,0,1,1,0,0,0,1,1,1,1,1,0,1,1};
// You need to change to your own gate ID!

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


void setup() {
  Serial.begin(115200);
  pinMode(OUTVIN, OUTPUT);
  pinMode(OUTGND, OUTPUT);
  pinMode(OUTDATA, OUTPUT);
  pinMode(LED, OUTPUT);
  digitalWrite(OUTVIN,HIGH);
  digitalWrite(OUTGND,LOW);
  digitalWrite(OUTDATA,LOW);

  // 'randomly' change 'rolling key'
  randomSeed(EEPROM.read(0));
  EEPROM.write(0,(byte)random(255));
  EEPROM.write(1,(byte)random(255));
  byte r0 = EEPROM.read(0);
  byte r1 = EEPROM.read(1);
  for (int j=0;j<8;j++) {
    signal433[36+j] = ((r0>>j)&1);
    signal433[44+j] = ((r1>>j)&1);
  };
 
}

void loop() {
  Serial.println(analogRead(0));
  int i;
  int t = 0;
  unsigned int ttime = SIGNAL_PERIOD/3; 
    
  for (t=0; t<REPEATSIGNAL;t++){   
      //SEND PREAMBLE
      SENDWAIT(1,SIGNAL_PREAMBLE)
      for (i = 0; i < signalLength; i++) {
        
         // There is a 66% (active low) duty cycle for '1' and 33% for '0'
         // Which rougly translate to: 
         // 001 -> encodes '1' 
         // 011 -> encodes '0'
        
        SENDWAIT(0,ttime);
        SENDWAIT(!signal433[i], ttime);
        SENDWAIT(1,ttime);
      }
      //WAIT SOME INTERVAL
      SENDWAIT(0,SIGNAL_INTERVAL); 
  }
  
  // A PAUSE AND A BLINK
  blinky();
  delay(2000);
}