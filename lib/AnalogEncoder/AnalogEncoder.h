#ifndef ANALOG_ENCODER_H
#define ANALOG_ENCODER_H

#include <Arduino.h>
#include <VelocityModule.h>

class AnalogEncoder{
public:
  void begin();
  void processAnalogEncoder();
  int32_t getCount();
  int32_t getVelocity();
  void reset();
private:
  const int analogPin = PA1;
  VelocityModule velocityModule;
  uint16_t lastEncoderLowBits = 0;
  int32_t encoderHighBits = 0;
};

extern AnalogEncoder analogEncoder;

#endif
