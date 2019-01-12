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
  if (index >= WINDOW_SIZE){
    index = 0;
  }
}

uint16_t SupplyReader::getRawReading(){
  int16_t lastIndex = index - 1;
  if(lastIndex < 0){
    lastIndex = WINDOW_SIZE - 1;
  }
  return samples[lastIndex];
}

uint16_t SupplyReader::getReading(){
  uint32_t sum = 0;
  for(uint8_t i = 0; i < WINDOW_SIZE; i++){
    sum += samples[i];
  }

  return sum/WINDOW_SIZE;
}

SupplyReader supplyReader;
