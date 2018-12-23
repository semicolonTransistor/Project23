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
  lastChange = millis();
}

void Blinker::update(){
  switch (mode) {
    case On:
    digitalWrite(pin, LOW);
    break;
    case Off:
    digitalWrite(pin, HIGH);
    break;
    case Blinking:
    if(millis() - lastChange > halfPeriod){
      digitalWrite(pin, ~digitalRead(pin));
      lastChange = millis();
    }
  }
}
