
#include "sensors_filtering.h"

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

bool LowPassFilterSensor::sense() {
  const unsigned long now = millis();
  if (now - lastReading >= period) {
    //TRACE_PRINT(F("reading sensor value: ")); 
    
    // let's read
    float v = float(analogRead(pin));
    //TRACE_PRINT(v);
    pastvalue = (int)(ETA * v + (1.0 - ETA) * float(pastvalue));
    //TRACE_PRINT(F(" => "));
    //TRACE_PRINTLN(pastvalue);
    lastReading = now;
    return true;
  }
  return false;
}


 
LowPassFilterSensorWithMinMax::LowPassFilterSensorWithMinMax(
                  const float _ETA, const byte _pin, const unsigned int _period, 
                  const float _ETAslow, const float _ETAquick)
             :LowPassFilterSensor(_ETA, _pin, _period) {

  ETAquick = _ETAquick;
  ETAslow = _ETAslow;
}

void LowPassFilterSensorWithMinMax::setup(int initialMin, int initialMax) {
  LowPassFilterSensor::setup();

  if (initialMin >= 0) {
    currentMin = float(initialMin);
  } else {
    currentMin = float(value())-5;
  }
  if (initialMax >= 0) {
    currentMax = float(initialMax);  
  } else {
    currentMax = float(value())+5;
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
  TRACE_PRINT(F(" max="));  TRACE_PRINTLN(currentMax);
  
}
bool LowPassFilterSensorWithMinMax::sense() {

  // measure the current value
  if (!LowPassFilterSensor::sense()) {
    return false;
  }

  float v = float(value());

  if (v >= currentMin) {
    currentMin = ETAslow * v + (1.0 - ETAslow)*currentMin;
  } else {
    currentMin = ETAquick * v + (1.0 - ETAquick)*currentMin;
  }
  
  if (v <= currentMax) {
    currentMax = ETAslow * v + (1.0 - ETAslow)*currentMax;
  } else {
    currentMax = ETAquick * v + (1.0 - ETAquick)*currentMax;
  }
  
  if (currentMin < 1) {
    currentMin = 1;
  }
  if (currentMax < 1) {
    currentMax = 1;
  }
  return true;
}



