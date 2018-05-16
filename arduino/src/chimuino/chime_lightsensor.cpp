#include "chime_lightsensor.h"

#include <Arduino.h>

#include "debug.h"

ChimeLightSensor::ChimeLightSensor(const char _pin):
              sensor(CHIME_LIGHTSENSOR_MEASURES_ETA, _pin, CHIME_LIGHTSENSOR_MEASURES_FREQUENCY)  
              {

  // save params
  pin = _pin;

}

void ChimeLightSensor::perceive() {
  sensor.sense();
}

void ChimeLightSensor::debugSerial() {
  DEBUG_PRINT("LIGHT LEVEL: "); DEBUG_PRINT(sensor.value()); 
  DEBUG_PRINTLN(isDark() ? "(dark)":"(lid)");
}

void ChimeLightSensor::setup() {
  
  pinMode(pin, INPUT); 
        
}

unsigned short ChimeLightSensor::getLightLevel() {
  return sensor.value();
}

bool ChimeLightSensor::isDark() {
  return getLightLevel() <= darkThreshold;
}






