
#include "sensors_filtering.h"

#include <Arduino.h>


LowPassFilterSensor::LowPassFilterSensor(float _ETA, char _pin, const unsigned int _period) {
  pin = _pin;
  pinMode(_pin, INPUT); 

  _ETA = ETA;
  period = _period;
  lastReading = millis();
  pastvalue = analogRead(pin);
}

bool LowPassFilterSensor::sense() {
  const unsigned long now = millis();
  if (now - lastReading > period) {
    // let's read
    pastvalue = ETA*analogRead(pin) + (1-ETA)*pastvalue;
    lastReading = now;
    return true;
  }
  return false;
}


 
LowPassFilterSensorWithMinMax::LowPassFilterSensorWithMinMax(
                  const float _ETA, const char _pin, const unsigned int _period, 
                  const float _ETAquick, const float _ETAslow)
             :LowPassFilterSensor(_ETA, _pin, _period) {

  currentMin = currentMax = value();
  
}

bool LowPassFilterSensorWithMinMax::sense() {

  // measure the current value
  if (!LowPassFilterSensor::sense()) {
    return false;
  }
  
  int v = value();
  
  if (v <= currentMin) {
    currentMin = ETAquick*v + (1 - ETAquick)*currentMin;
  } else {
    currentMin = ETAslow*v + (1 - ETAslow)*currentMin;
  }

  if (v >= currentMax) {
    currentMax = ETAquick*v + (1 - ETAquick)*currentMax;
  } else {
    currentMax = ETAslow*v + (1 - ETAslow)*currentMax;
  }

  return true;
}



