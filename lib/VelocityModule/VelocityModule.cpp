#include "VelocityModule.h"

void VelocityModule::addPositionSample(int32_t position){
  pastPositions[positionIndex] = position;
  positionIndex ++;
  if(positionIndex >= POSITION_WINDOW){
    positionIndex = 0;
  }
  int16_t pastIndex = positionIndex + 1;
  if(pastIndex >= POSITION_WINDOW) {
    pastIndex = 0;
  }
  int32_t velocity = position - pastPositions[pastIndex];
  pastVelocities[velocityIndex] = velocity;
  velocityIndex ++;
  if(velocityIndex >= VELOCITY_WINDOW){
    velocityIndex = 0;
  }
}

int32_t VelocityModule::getRawVelocity(){
  int16_t lastIndex = velocityIndex-1;
  if(lastIndex < 0){
    lastIndex = VELOCITY_WINDOW - 1;
  }
  return pastVelocities[lastIndex];
}

int32_t VelocityModule::getVelocity(){
  int64_t sum = 0;
  for(uint8_t i = 0; i < VELOCITY_WINDOW; i++){
    sum += pastVelocities[i];
  }
  return (int32_t)sum/VELOCITY_WINDOW;
}
