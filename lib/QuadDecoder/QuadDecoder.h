#ifndef QUAD_DECODER_H
#define QUAD_DECODER_H

#include <Arduino.h>
#include "VelocityModule.h"

class QuadDecoder{
public:
  void begin();
  void processDecoder();
  void reset();
  int32_t getCount();
  int32_t getVelocity();
private:
  int32_t encoderHighBits = 0;
  uint16_t lastEncoderLowBits = 0;
  VelocityModule velocityModule;
};

extern QuadDecoder quadDecoder;

#endif
