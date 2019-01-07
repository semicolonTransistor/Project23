#include "SupplyReader.h"

void SupplyReader::begin(){
  //set up internal refrence;
  adc_reg_map *regs = ADC1->regs;
  regs->CR2 |= ADC_CR2_TSVREFE;    // enable VREFINT and temp sensor
  regs->SMPR1 =  ADC_SMPR1_SMP17;  // sample rate for VREFINT ADC channel
}

void SupplyReader::processSupply(){
  uint16_t sample = adc_read(ADC1, 17);
  samples[index] = sample;
  index ++;
}

uint16_t SupplyReader::getRawReading(){
  return samples[index - 1];
}

uint16_t SupplyReader::getReading(){
  uint32_t sum = 0;
  for(uint8_t i = 1; i <= 10; i++){
    sum += samples[index - i];
  }

  return sum/10;
}

SupplyReader supplyReader;
