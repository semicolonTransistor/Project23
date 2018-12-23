#ifndef BLINKER_H
#define BLINKER_H

#include <Arduino.h>

enum BlinkerMode {On, Off, Blinking};

class Blinker{
protected:
  uint32_t halfPeriod = 0;
  uint32_t lastChange = 0;
  int pin;
  int state = LOW;
  BlinkerMode mode = Off;
public:
  void attach(int pinNumber);
  void set(BlinkerMode mode);
  void interval(uint32_t period);
  void update();


};

#endif
