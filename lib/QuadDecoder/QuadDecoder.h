#ifndef QUAD_DECODER_H
#define QUAD_DECODER_H

#include <Arduino.h>

class QuadDecoder{
public:
  void begin();
  void processDecoder();
  int32_t getCount();
  int32_t getVelocity();
private:
  int32_t encoderHighBits = 0;
  uint16_t lastEncoderLowBits = 0;
  int32_t pastEncoderCounts [256] = {};
  uint8_t pastEncoderCountsIndex = 0;
};

extern QuadDecoder quadDecoder;

#endif
