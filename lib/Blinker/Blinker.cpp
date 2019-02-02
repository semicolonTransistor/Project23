#include "Blinker.h"

void Blinker::attach(int pinNumber, uint8_t active){
  activeState = active;
  if(activeState == LOW){
    inactiveState = HIGH;
  }else{
    inactiveState = LOW;
  }
  state = inactiveState;
  pin = pinNumber;
  pinMode(pin, OUTPUT);
}

void Blinker::interval(uint32_t period){
  halfPeriod = period/2;
}

void Blinker::set(BlinkerMode mode){
  this->mode = mode;
}

void Blinker::toggle(){
  if(state == activeState){
    digitalWrite(pin, inactiveState);
    state = inactiveState;
  }else{
    digitalWrite(pin, activeState);
    state = activeState;
  }
}

void Blinker::update(){
  switch (mode) {
    case On:
    //turn on the output and sets the current state to on
    digitalWrite(pin, activeState);
    state = activeState;
    break;
    case Off:
    //turn off the output and sets the current state to off
    digitalWrite(pin, inactiveState);
    state = inactiveState;
    break;
    case Blinking:
    //toggels the output every half period
    if(millis() - lastChange > halfPeriod){
      toggle();
      lastChange = millis();
    }
  }
}
