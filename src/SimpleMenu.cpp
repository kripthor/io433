#include "SimpleMenu.h"


/////////////////////////////////////////////////////////////////
SimpleMenu::SimpleMenu(char* value) {
  name = value;  
  parent = NULL;
}

SimpleMenu::SimpleMenu(char* value, SimpleMenu *par) {
  name = value;  
  parent = par;
  par->addChild(this);
}

SimpleMenu::SimpleMenu(char* value, SimpleMenu *par, Callback action) {
  name = value;  
  parent = par;
  actionSelect = action;
  par->addChild(this);
}

SimpleMenu::SimpleMenu(char* value, SimpleMenu *par, Callback action, Callback actionend) {
  name = value;  
  parent = par;
  actionSelect = action;
  actionEnd = actionend;
  par->addChild(this);
}

/////////////////////////////////////////////////////////////////
void SimpleMenu::setParent(SimpleMenu *par){
  parent = par;  
}

/////////////////////////////////////////////////////////////////
void SimpleMenu::addChild(SimpleMenu *par){
  if (ci < _SM_MAXCHILD) {
    child[ci++] = par;  
    par->setParent(this);
  }
}

int SimpleMenu::getChildNum(){
  return ci;
}; 
  

/////////////////////////////////////////////////////////////////  
void SimpleMenu::setAction(Callback action){
  actionSelect = action;
}

////////////////////////////////////////////////////////////////
void SimpleMenu::setSelectedChild(int i) {
  if ((i >= 0) && (i < ci)) selected = i;
}

int SimpleMenu::getSelectedChild() {
 return selected; 
}
