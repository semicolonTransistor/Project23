#include "QuadDecoder.h"

void QuadDecoder::begin(){
  //init encoder pins.
  pinMode(PA8,INPUT_PULLUP);
  pinMode(PA9,INPUT_PULLUP);

  //register poking, setting up timer 1 in encoder mode
  TIMER1->regs.bas->CR1 = 0x0000;
  TIMER1->regs.bas->ARR = 0xFFFF;
  TIMER1->regs.bas->PSC = 0x0000;
  TIMER1->regs.adv->RCR = 0x0000;
  TIMER1->regs.bas->EGR = 0x0001;

  TIMER1->regs.gen->CCER = 0x0011;
  TIMER1->regs.gen->CCMR1 = 0xF1F1;

  TIMER1->regs.adv->SMCR = 0x0003;
  //enable TIMER1
  TIMER1->regs.bas->CR1 = 0x0001;
}

int32_t QuadDecoder::getCount(){
  return (encoderHighBits * 0x10000) + lastEncoderLowBits;
}

void QuadDecoder::processDecoder(){
  uint16_t encoderLowBits = TIMER1->regs.bas->CNT;
	if(encoderLowBits > lastEncoderLowBits){
		if(encoderLowBits - lastEncoderLowBits > 0x7FFF){
			encoderHighBits --;
		}
	}else{
		if(lastEncoderLowBits - encoderLowBits > 0x7FFF){
			encoderHighBits ++;
		}
	}
  lastEncoderLowBits = encoderLowBits;

  velocityModule.addPositionSample(getCount());
}

int32_t QuadDecoder::getVelocity(){
  noInterrupts();
  int32_t velocity = velocityModule.getVelocity();
  interrupts();
  return velocity;
}

QuadDecoder quadDecoder;
