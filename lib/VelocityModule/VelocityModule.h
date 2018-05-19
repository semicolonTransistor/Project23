#ifndef VELOCITY_MODULE_H
#define VELOCITY_MODULE_H

#include <Arduino.h>

class VelocityModule{
public:
  void addPositionSample(int32_t position);
  int32_t getVelocity();
  int32_t getRawVelocity();
private:
  int32_t pastPositions[256] = {};
  int32_t pastVelocities[256] = {};
  uint8_t pastPositionIndex = 0;
  uint8_t pastVelocityIndex = 0;
};


#endif
