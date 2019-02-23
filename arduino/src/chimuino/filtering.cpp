
#include "filtering.h"

#include <Arduino.h>

#include "debug.h"

LowPassFilterSensor::LowPassFilterSensor(float _ETA, byte _pin, const unsigned int _period) {
  pin = _pin;
  ETA = _ETA;
  period = _period;
  
}

void LowPassFilterSensor::setup() {
  
  pinMode(pin, INPUT); 

  lastReading = millis();
  pastvalue = analogRead(pin);
  
}

uint16_t LowPassFilterSensor::update_value(uint16_t v, uint16_t pastvalue, float ETA) {
    
    return update_value(float(v), pastvalue, ETA);
}

uint16_t LowPassFilterSensor::update_value(float vf, uint16_t pastvalue, float ETA) {

    return (uint16_t)((ETA * vf + (1000.0 - ETA) * float(pastvalue))/1000.0);
}

bool LowPassFilterSensor::sense() {
  const unsigned long now = millis();
  if (now - lastReading >= period) {
    
    // let's read
    pastvalue = update_value(uint16_t(analogRead(pin)), pastvalue, ETA);
    lastReading = now;
    return true;
  }
  return false;
}
 
LowPassFilterSensorWithMinMax::LowPassFilterSensorWithMinMax(
                  const float _ETA, const byte _pin, const unsigned int _period, 
                  const float _ETAslow, const float _ETAquick
                  ):LowPassFilterSensor(_ETA, _pin, _period) {

  ETAquick = _ETAquick;
  ETAslow = _ETAslow;
}

void LowPassFilterSensorWithMinMax::setup(uint8_t initialMin, uint8_t initialMax) {
  LowPassFilterSensor::setup();

  if (initialMin >= 0) {
    currentMin = initialMin;
  } else {
    currentMin = value()-5;
  }
  if (initialMax >= 0) {
    currentMax = initialMax;  
  } else {
    currentMax = value()+5;
  }
  
  if (currentMin < 1) {
    currentMin = 1;  
  }
  if (currentMax < 1) {
    currentMax = 1;
  }
  if (currentMax < currentMin) {
    currentMax = currentMin + 5;
  }

  TRACE_PRINT(F("init min="));  TRACE_PRINT(currentMin);
  TRACE_PRINT(F(" max="));      TRACE_PRINTLN(currentMax);
  
}

void LowPassFilterSensorWithMinMax::adaptMinMax(float v) {
  
  if (v >= currentMin) {
    currentMin = update_value(v, currentMin, ETAslow);
  } else {
    currentMin = update_value(v, currentMin, ETAquick);
  }
  
  if (v <= currentMax) {
    currentMax = update_value(v, currentMax, ETAslow);
  } else {
    currentMax = update_value(v, currentMax, ETAquick);
  }
  
  if (currentMin < 1) {
    currentMin = 1;
  }
  if (currentMax < 1) {
    currentMax = 1;
  }
}

bool LowPassFilterSensorWithMinMax::sense() {

  // measure the current value
  if (!LowPassFilterSensor::sense()) {
    return false;
  }

  adaptMinMax(value());
  
  return true;
}
