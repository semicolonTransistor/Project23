#include "AnalogEncoder.h"

void AnalogEncoder::begin(){
  pinMode(analogPin, INPUT_ANALOG);
}

int32_t AnalogEncoder::getCount(){
  return (encoderHighBits * 0x1000) + lastEncoderLowBits;
}

void AnalogEncoder::processAnalogEncoder(){
  uint16_t encoderLowBits = analogRead(analogPin);

  if(encoderLowBits > lastEncoderLowBits){
		if(encoderLowBits - lastEncoderLowBits > 0x07FF){
			encoderHighBits --;
		}
	}else{
		if(lastEncoderLowBits - encoderLowBits > 0x07FF){
			encoderHighBits ++;
		}
	}
  lastEncoderLowBits = encoderLowBits;

  velocityModule.addPositionSample(getCount());
}

int32_t AnalogEncoder::getVelocity(){
  noInterrupts();
  int32_t velocity = velocityModule.getVelocity();
  interrupts();
  return velocity;
}

void AnalogEncoder::reset(){
  encoderHighBits = 0;
}

AnalogEncoder analogEncoder;
