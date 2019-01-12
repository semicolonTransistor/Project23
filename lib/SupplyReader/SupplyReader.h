#ifndef SUPPLY_READER_H
#define SUPPLY_READER_H

#include <Arduino.h>

#define WINDOW_SIZE 10

class SupplyReader{
public:
  void begin();
  void processSupply();
  uint16_t getRawReading();
  uint16_t getReading();
protected:
  uint8_t index;
  uint16_t samples[WINDOW_SIZE] = {1470};
};

extern SupplyReader supplyReader;

#endif
