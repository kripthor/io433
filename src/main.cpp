#include "spiffsutils.h"
#include "CC1101utils.h"
#include "SimpleMenuNav.h"
#include "WiFi.h"
#include "esp_sleep.h"
#include "esp_bt_main.h"
#include "esp_bt.h"
#include "esp_wifi.h"
#include "driver/adc.h"


#define LOOPDELAY 20
#define HIBERNATEMS 30*1000
#define BUFSIZE 2048
#define REPLAYDELAY 0
// THESE VALUES WERE FOUND PRAGMATICALLY
#define RESET443 64000 //32ms
#define WAITFORSIGNAL 32 // 32 RESET CYCLES
#define MINIMUM_TRANSITIONS 32
#define MINIMUM_COPYTIME_US 16000
#define DUMP_RAW_MBPS 0.1 // as percentage of 1Mbps, us precision. (100kbps) This is mainly to dump and analyse in, ex, PulseView
#define BOUND_SAMPLES false

//ONLY USING ONE BUFFER FOR NOW, MUST BE REFACTORED TO SUPPORT MORE (AND MOVE TO SPIFFS)
uint16_t signal433_store[MAXSIGS][BUFSIZE];
uint16_t *signal433_current = signal433_store[0];


int delayus = REPLAYDELAY;
long lastCopyTime = 0;

int trycopy() {
  int i;
  Serial.println("Copying...");
  uint16_t newsignal433[BUFSIZE];
  memset(newsignal433,0,BUFSIZE*sizeof(uint16_t));
  byte n = 0;
  int64_t startus = esp_timer_get_time();
  int64_t startread;
  int64_t dif = 0;
  int64_t ttime = 0;
  for (i = 0; i < BUFSIZE; i++) {
    startread = esp_timer_get_time();
    dif = 0;
    //WAIT FOR INIT
    while (dif < RESET443) {
      dif = esp_timer_get_time() - startread;
      if (CCAvgRead() != n) {
        break;
      }
    }
    if (dif >= RESET443) {
       newsignal433[i] = RESET443;
      //if not started wait...
      if (i == 0) {
        i = -1;
        ttime++;
        if (ttime > WAITFORSIGNAL) {
          Serial.println("No signal detected!");
          return -1;
        }
      }
      else {
        ttime++;
        if (ttime > WAITFORSIGNAL) {
          Serial.println("End of signal detected!");
          break;
        }
        /*
        Serial.println("End of signal!");
        break;*/
      }
    }
    else {
     newsignal433[i] = dif;
     n = !n;
    }
  }
  
  int64_t stopus = esp_timer_get_time();
  Serial.print("Copy took (us): ");
  lastCopyTime = (long)(stopus - startus);
  Serial.println(lastCopyTime , DEC);
  Serial.print("Transitions: ");
  Serial.println(i);
  memcpy(signal433_current,newsignal433,BUFSIZE*sizeof(uint16_t));
  return i;
}

void copy() {
  int i, transitions = 0;
  lastCopyTime = 0;
  CCInit();
  CCSetRx();
  delay(50);
  //FILTER OUT NOISE SIGNALS (too few transistions or too fast)
  while (transitions < MINIMUM_TRANSITIONS && lastCopyTime < MINIMUM_COPYTIME_US) {
    transitions = trycopy();
    if (SMN_isUpButtonPressed()) return;
  }
  //CLEAN LAST ELEMENTS
  for (i=transitions-1;i>0;i--) {
    if (signal433_current[i] == RESET443) signal433_current[i] = 0;
    else break;
  }
  if (BOUND_SAMPLES) {
    signal433_current[0] = 200;
    if (i < BUFSIZE) signal433_current[i+1] = 200;
  }
  
  String fname = "/" + String(pcurrent) +".bin";
  storeSPIFFS(fname.c_str(),signal433_current,BUFSIZE);
}



void replay (int t) {
  int i;
  unsigned int totalDelay = 0;
  CCInit();
  CCSetTx();
  delay(50);
  int64_t startus = esp_timer_get_time();
  while (t-- > 0) {
    byte n = 0;
    for (i = 0; i < BUFSIZE; i++) {
      CCWrite(n);
      totalDelay = signal433_current[i]+delayus;
      delayMicroseconds(totalDelay);
      if (signal433_current[i] < RESET443) n = !n;
    }
     CCWrite(0);
  }
  CCSetRx();
  
  int64_t stopus = esp_timer_get_time();
  Serial.print("Replay took (us): ");
  Serial.println((long)(stopus - startus), DEC);

}


void replay () {
  replay(1);
}

void dump () {
  long ttime = 0;
  int trans = 0;
  int i,j;
  int n = 0;
  Serial.println("Dump transition times: ");
  for (i = 0; i < BUFSIZE; i++) {
    if (signal433_current[i] <= 0) break;
    if (i > 0) Serial.print(",");
    Serial.print(signal433_current[i]);
    ttime += signal433_current[i];
    if (signal433_current[i] != RESET443) {
      n = !n;
      trans++;
    }
  }
  Serial.print("\nTotal time (us): ");
  Serial.println(ttime, DEC);
  Serial.print("Transitions 0/1: ");
  Serial.println(trans, DEC);
  
  Serial.print("Dump raw (");
  Serial.print(DUMP_RAW_MBPS);
  Serial.println("Mbps):");
  
  n = 0;
  long samples = 0;
  for (i = 0; i < BUFSIZE; i++) {
    if (signal433_current[i] <= 0) break;
    for (j = 0; j < signal433_current[i]*DUMP_RAW_MBPS; j++) {
       samples++;
       if (n) Serial.write(124);
       else Serial.write(46);
       Serial.flush();
    }
    if (signal433_current[i] != RESET443) n = !n;
  }
  Serial.print("\nTotal samples: ");
  Serial.println(samples);
  SMN_dump(signal433_current,BUFSIZE,RESET443);
 
}


// THIS IS OBVIOUSLY SLOW
void rawout() {
  CCInit();
  CCSetRx();
  delay(50);

  byte b;
  bool endloop = false;
  long t = 0;
  long start = millis();
  while(!endloop) {
    b = CCAvgRead();
    //BOTTLENECK ON SERIAL WRITE... OUT VALUES ARE BOTH ASCII AND EASY TO FILTER IN PULSEVIEW
    if (b) Serial.write(124);
    else Serial.write(46);
    Serial.flush();
    t++;
    if (SMN_isAnyButtonPressed()) {
      endloop = true;
    }
  }
  long tt = millis() - start;
  
  Serial.print("\n Total time(ms): ");
  Serial.print(tt);
  Serial.print("\n Total bits: ");
  Serial.print(t);
  Serial.print("\n bits/sec: ");
  Serial.println((t*1.0)/(tt/1000.0));
}

// THIS IS OBVIOUSLY NOT REAL TIME
void monitormode() {
  CCInit();
  CCSetRx();
  delay(50);
 
  int k = 1;
  int i = 0;
  int rssi = 0;
  int maxrssi = -999;
  int minrssi = 0;
  int oldy = 0;
  int newy = 0;

  tft.fillScreen(TFT_BLACK);
  tft.drawRect(0, 0, WIDTH-1, HEIGHT-1, TFT_WHITE);
  tft.setFreeFont(FMB9);
  tft.setTextColor(TFT_RED, TFT_WHITE);
  
  delay(200);
  while(true) {
    i = CCAvgRead();
    tft.drawLine(k, HEIGHT/2, k, HEIGHT-16, TFT_BLACK);
    if (i) newy =  HEIGHT/2;
    else newy = HEIGHT-16;
    tft.drawLine(k,oldy,k,newy, TFT_GREEN);
    oldy=newy;
    if (k%50 == 0) rssi = ELECHOUSE_cc1101.getRssi();
    maxrssi = max(maxrssi,rssi);
    minrssi = min(minrssi,rssi);
    delayMicroseconds(200);
    if (k++ >= WIDTH-5) {
      tft.drawString(" RSSI: M      m      ",7,10 , GFXFF);
      tft.drawString(String(maxrssi),10*10,10 , GFXFF);
      tft.drawString(String(minrssi),18*10,10 , GFXFF);
      k = 1;
      maxrssi = -999;
      minrssi = 0;
      if (SMN_isUpButtonPressed()) return;
      while (SMN_isDownButtonPressed()) delay(100);
    }
  }
}


void setup() {
  
  CCInit();
  CCSetRx();
  Serial.begin(1000000);

  ////////////// DEFINE THE MENUS ////////////////
  /*
  MAIN
  |-> COPY
  |-> REPLAY
  |-> DUMP
  |-> MORE
      |-> MONITOR
      |-> RAW OUT
      |-> ABOUT
  */

  SimpleMenu *menu_main = new SimpleMenu("Main");
  SimpleMenu *menu_copy = new SimpleMenu("Copy",menu_main,copy);
  SimpleMenu *menu_replay = new SimpleMenu("Replay",menu_main,replay);
  SimpleMenu *menu_dump = new SimpleMenu("Dump",menu_main,dump);
  SimpleMenu *menu_more = new SimpleMenu("More",menu_main,NULL);

  SimpleMenu *menu_monitor = new SimpleMenu("Monitor",menu_more,monitormode);
  SimpleMenu *menu_load = new SimpleMenu("Raw Out",menu_more,rawout);
  SimpleMenu *menu_about = new SimpleMenu("About",menu_more,SMN_screensaver);


  menu_dump->alertDone = false;
  menu_monitor->alertDone = false;
  menu_about->alertDone = false;
  SMN_initMenu(menu_main);
 
  //// ENSURE RADIO OFF (FOR LESS INTERFERENCE?)
  esp_bluedroid_disable();
  esp_bt_controller_disable();
  esp_wifi_stop();
  //adc_power_off();

  if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)){
    Serial.println("SPIFFS Mount Failed");
    return;
  }

  Serial.println("List SPIFFS:");
  listSPIFFS("/", 1);
  Serial.println("Loading files...");
  
  for (int f=0;f<MAXSIGS;f++) {
    String fname = "/" + String(f) +".bin";
    loadSPIFFS(fname.c_str(),signal433_store[f],BUFSIZE);
  }
  

  //NEED TO DOUBLE CHECK THIS MATH
  String vbat = String((float)( analogRead(34) / 4095.0 * 2 * 3.3 * 1.1));
  SMN_alert("Bat = \n"+vbat+"v",100,1500);
}


void loop() {
  SMN_loop(); //MUST BE REGULARY CALLED.
  delay(LOOPDELAY);
  signal433_current = signal433_store[pcurrent];

  if (SMN_idleMS() > HIBERNATEMS) {
    SMN_alert("SLEEPING...",100,3000);
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_0,0);
    
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_AUTO);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_AUTO);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_AUTO);
    esp_sleep_pd_config(ESP_PD_DOMAIN_XTAL, ESP_PD_OPTION_AUTO);
    ELECHOUSE_cc1101.goSleep();
    esp_deep_sleep_start();
  }
}
