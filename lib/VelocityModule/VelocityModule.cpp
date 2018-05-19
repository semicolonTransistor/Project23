#include "VelocityModule.h"

void VelocityModule::addPositionSample(int32_t position){
  pastPositions[pastPositionIndex] = position;
  pastPositionIndex ++;
  uint8_t pastIndex = pastPositionIndex-(uint8_t)100;
  int32_t velocity = position - pastPositions[pastIndex];
  pastVelocities[pastVelocityIndex] = velocity;
  pastVelocityIndex ++;
}

int32_t VelocityModule::getRawVelocity(){
  return pastVelocities[pastVelocityIndex-1];
}

int32_t VelocityModule::getVelocity(){
  int64_t sum = 0;
  for(uint8_t i = 1; i <= 64; i++){
    uint8_t index = pastVelocityIndex-i;
    sum += pastVelocities[index];
  }
  return (int32_t)sum/64;
}
