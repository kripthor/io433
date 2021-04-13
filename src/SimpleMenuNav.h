#include "SimpleMenu.h"
#include "Button2.h"
#include "Free_Fonts.h"
#include <TFT_eSPI.h> 


#define BUTTON_UP  35
#define BUTTON_DOWN  0
#define LONGCLICK_MS 300
#define DOUBLECLICK_MS 300

#define WIDTH  240
#define HEIGHT 135

Button2 butUp = Button2(BUTTON_UP);
Button2 butDown = Button2(BUTTON_DOWN);

bool needsRefresh = true;
bool needsExitFromAction = false;
long lastClick;

//TODO: REFACTOR
#define MAXSIGS 10
int pcurrent = 0;



SimpleMenu* active_menu = NULL;
TFT_eSPI tft = TFT_eSPI();

void SMN_printAt(String s, int x, int y) {
  tft.drawString(s, x, y, GFXFF);
  Serial.println(s);
}

void SMN_alert(String msg, int wait, int timeout) {
  delay(wait);
  tft.setFreeFont(FMB12);
  tft.setTextColor(TFT_MAROON, TFT_DARKGREY);
  tft.fillRect(20, 20, WIDTH-40, HEIGHT-40, TFT_DARKGREY);
  SMN_printAt("X", WIDTH-42, 22); 
  SMN_printAt(msg, 40, HEIGHT/2-10); 
  if (timeout > 0) {
    delay(timeout);
    needsRefresh = true;
  }
}

void SMN_handler(Button2& btn) {
    //Serial.println(String(btn.getClickType()) + " click type");
    lastClick = millis();
    needsRefresh = true;
    if (needsExitFromAction) {
       needsExitFromAction = false;
       return;
    } 
    switch (btn.getClickType()) {
        case SINGLE_CLICK:
            lastClick = millis();
            if (btn.getAttachPin() == BUTTON_UP) {
              int c = active_menu->getSelectedChild();
              if (c > 0) active_menu->setSelectedChild(--c);
            } else {
              int c = active_menu->getSelectedChild();
              if (c < _SM_MAXCHILD) active_menu->setSelectedChild(++c);
           }
            break;
        case LONG_CLICK:
            if (btn.getAttachPin() == BUTTON_UP) {
              if (active_menu->parent != NULL) active_menu = active_menu->parent;
            } else {
              int c = active_menu->getSelectedChild();
              SimpleMenu* clicked_menu = active_menu->child[c];
              if (clicked_menu->getChildNum() > 0) {
                active_menu = clicked_menu;
              } else {
                if (clicked_menu->actionSelect != NULL) {
                   needsRefresh = false;
                   needsExitFromAction = true;
                   tft.fillScreen(TFT_BLACK);
                   tft.drawRect(0, 0, WIDTH-1, HEIGHT-1, TFT_WHITE);
                   tft.setFreeFont(FMB24);
                   tft.setTextColor(TFT_RED, TFT_BLACK);
                   SMN_printAt(clicked_menu->name, 20, 40); 
                   
                   if (clicked_menu->actionSelect != NULL) clicked_menu->actionSelect();
                   if (clicked_menu->alertDone) SMN_alert(String(clicked_menu->name)+" done!",500,0);
                   
                 }
              }
           }
            break;
        case DOUBLE_CLICK:
            if (btn.getAttachPin() == BUTTON_UP) {
              pcurrent ++;
              if (pcurrent >= MAXSIGS) pcurrent = 0;
            }
            else {
              pcurrent --;
              if (pcurrent < 0) pcurrent = MAXSIGS -1;
            }
            break;
        default:
            Serial.println(String(btn.getClickType()) + " click type / Not implemented ");
            break;
    }
}

bool SMN_isAnyButtonPressed() {
  return (butUp.isPressedRaw() || butDown.isPressedRaw());
}
bool SMN_isUpButtonPressed() {
  return butUp.isPressedRaw();
}
bool SMN_isDownButtonPressed() {
  return butDown.isPressedRaw();
}

void SMN_screensaver() {
  byte xx[24];
  memset(xx,0,24*sizeof(byte));
  byte col,i,x,y;
  char c;
  while(true) {
    col = random(24);
    for (i=0;i<=xx[col];i++) {
      x = col*10;
      y = i*12+1;
      if ((i%2==0)^(col%2==0)) c = '1'; else c = '0';
      tft.setFreeFont(FM9);
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.drawChar(c, x, y, GFXFF);
      delay(15);
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
      tft.drawChar(c, x, y, GFXFF);
      if (SMN_isAnyButtonPressed()) return;
    }
    xx[col]++;
    if (xx[col] > 11) xx[col] = 0;
    tft.setFreeFont(FM12);
    tft.setTextColor(TFT_MAROON, TFT_DARKGREY);
    tft.drawString("@kripthor", 100, 110, GFXFF);
  }
}

void SMN_initMenu(SimpleMenu *menu) {
  butUp.setLongClickTime(300);
  butUp.setDoubleClickTime(300);
  butUp.setClickHandler(SMN_handler);
  butUp.setLongClickHandler(SMN_handler);
  butUp.setDoubleClickHandler(SMN_handler);
 
  butDown.setLongClickTime(300);
  butDown.setDoubleClickTime(300);  
  butDown.setClickHandler(SMN_handler);
  butDown.setLongClickHandler(SMN_handler);
  butDown.setDoubleClickHandler(SMN_handler);
 
  active_menu = menu;

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.setFreeFont(FMB9);
  SMN_printAt(" ___ ___  _ _ ________",0,26-16);
  SMN_printAt("|_ _/ _ \\| | |__ /__ /",0,26*2-16);
  SMN_printAt(" | | (_) |_  _|_ \\|_ \\",0,26*3-16);
  SMN_printAt("|___\\___/  |_|___/___/",0,26*4-16);
  delay(1000);
 
}

void SMN_printMenu() {
  tft.fillScreen(TFT_BLACK);

  tft.setFreeFont(FMB18);         
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  SMN_printAt(active_menu->name, 10, 2);

  tft.setTextColor(TFT_RED, TFT_BLACK);
  String p = "[";
  p+=pcurrent;
  p+="]";
  SMN_printAt(p, 180, 2);
 
  tft.setFreeFont(FMB12);         
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
 
  for (int i=0; i < active_menu->getChildNum(); i++) {
    String entry = "";
    if (active_menu->getSelectedChild() == i) entry += "(*)";  
    else entry += "( )";
    entry += active_menu->child[i]->name;  
    SMN_printAt(entry,10,34+i*24); 
  }
  
  tft.drawLine(0, 31, WIDTH-1, 31, TFT_GREEN);
  tft.drawRect(0, 0, WIDTH-1, HEIGHT-1, TFT_YELLOW);
  needsRefresh = false;
}


void SMN_loop() {
  butUp.loop();
  yield();
  butDown.loop();
  yield();
  if (needsRefresh) {
    //Serial.println("Refreshing screen");
    SMN_printMenu();
  }
  yield();
}


////////////// CUSTOM SCREENS /////////////

void SMN_copy() {
  tft.fillScreen(TFT_BLACK);
  tft.drawRect(0, 0, WIDTH-1, HEIGHT-1, TFT_YELLOW);
  tft.setFreeFont(FMB24);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  SMN_printAt("COPY...", 20, 30);   
}


void drawRightArrow(int x1,int y1,int x2,int y2,int color) {
  tft.drawLine(x1, y1, x2, y2, color);
  tft.drawLine(x2, y2, x2-5,y2-3, color);
  tft.drawLine(x2, y2, x2-5,y2+3, color);
  tft.drawLine(x2/2, y2, x2/2-5,y2-3, color);
  tft.drawLine(x2/2, y2, x2/2-5,y2+3, color);
}

void SMN_dump(uint16_t signal433[], int bufsize, int maxt) {
  int i;
  
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setFreeFont(FM9);
  tft.drawRect(0, 0, WIDTH-1, HEIGHT-1, TFT_YELLOW);
  
  /*for (i = WIDTH/5; i < WIDTH-1; i+=WIDTH/5) {
    tft.drawLine(i, 20, i, (int)(HEIGHT/2.0)-6, TFT_DARKGREY);
  }*/
  tft.drawLine(4, 22, 4, 30, TFT_ORANGE);
  drawRightArrow(4, 26, 236, 26, TFT_ORANGE);
  tft.drawLine(236, 26, 4, 79, TFT_ORANGE);
  drawRightArrow(4, 79, 236, 79, TFT_ORANGE);  
  
  long total = 0;
  int trans = 0;
  int n = 0;
  int x1 = 0,x2 = 0,y = 0;

  for (i = 0; i < bufsize; i++) {
    if (signal433[i] <= 0) break;
     total += signal433[i];
  }
  trans = i;
  if (trans <= 0) return;
  long avg = total/trans;
  avg /= 4;  
    
  long loctotal = 0;
  // Split into 2 lines
  for (i = 0; i < trans-1; i++) {
    x2 = (int)(signal433[i] / avg);
    loctotal += signal433[i];
    if (x1+x2 >= WIDTH-1) break;
    if (n) y = 32;
    else y = 72;
    tft.drawLine(x1, y, x1+x2, y, TFT_GREEN);
    if (signal433[i] != maxt) {
      tft.drawLine(x1+x2, 32, x1+x2, 72, TFT_GREEN);
      n = !n;
    }
    x1+=x2;

  }
  
  int j;
  x1 = x2 = y = 0;
  for (j = i; j < trans-1; j++) {
    x2 = (int)(signal433[j] / avg);
    loctotal += signal433[j];
    if (x1+x2 >= WIDTH-1) break;
    if (n) y = 86;
    else y = 126;
    tft.drawLine(x1, y, x1+x2, y, TFT_GREEN);
    if (signal433[i] != maxt) {
      tft.drawLine(x1+x2, 86, x1+x2, 126, TFT_GREEN);
      n = !n;
    }
    x1+=x2;
  }
  
  String tus = String(loctotal) + "us of "+String(total);
  tus += "us";  
  SMN_printAt(tus, 2, 2);   
  
}
