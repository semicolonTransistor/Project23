#include "Blinker.h"

void Blinker::attach(int pinNumber){
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
  if(state == LOW){
    digitalWrite(pin, HIGH);
    state = HIGH;
  }else{
    digitalWrite(pin, LOW);
    state = LOW;
  }
}

void Blinker::update(){
  switch (mode) {
    case On:
    digitalWrite(pin, LOW);
    state = LOW;
    break;
    case Off:
    digitalWrite(pin, HIGH);
    state = HIGH;
    break;
    case Blinking:
    if(millis() - lastChange > halfPeriod){
      toggle();
      lastChange = millis();
    }
  }
}
