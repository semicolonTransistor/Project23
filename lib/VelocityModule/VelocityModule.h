#ifndef VELOCITY_MODULE_H
#define VELOCITY_MODULE_H

#include <Arduino.h>

#define VELOCITY_WINDOW 64
#define POSITION_WINDOW 101

class VelocityModule{
public:
  void addPositionSample(int32_t position);
  int32_t getVelocity();
  int32_t getRawVelocity();
private:
  int32_t pastPositions[POSITION_WINDOW] = {};
  int32_t pastVelocities[VELOCITY_WINDOW] = {};
  int16_t positionIndex = 0;
  int16_t velocityIndex = 0;
};


#endif
