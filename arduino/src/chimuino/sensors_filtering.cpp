
#include "sensors_filtering.h"

#include <Arduino.h>

#include "debug.h"

LowPassFilterSensor::LowPassFilterSensor(float _ETA, byte _pin, const unsigned int _period) {
  pin = _pin;
  pinMode(_pin, INPUT); 

  ETA = _ETA;
  period = _period;
  lastReading = millis();
  pastvalue = analogRead(pin);
}

bool LowPassFilterSensor::sense() {
  const unsigned long now = millis();
  if (now - lastReading >= period) {
    //DEBUG_PRINT(F("reading sensor value: ")); 
    
    // let's read
    float v = float(analogRead(pin));
    //DEBUG_PRINT(v);
    pastvalue = (int)(ETA * v + (1.0 - ETA) * float(pastvalue));
    //DEBUG_PRINT(F(" => "));
    //DEBUG_PRINTLN(pastvalue);
    lastReading = now;
    return true;
  }
  return false;
}


 
LowPassFilterSensorWithMinMax::LowPassFilterSensorWithMinMax(
                  const float _ETA, const byte _pin, const unsigned int _period, 
                  const float _ETAquick, const float _ETAslow)
             :LowPassFilterSensor(_ETA, _pin, _period) {

  ETAquick = _ETAquick;
  ETAslow = _ETAslow;
  
  currentMin = 1;
  currentMax = value();
    
}

bool LowPassFilterSensorWithMinMax::sense() {

  // measure the current value
  if (!LowPassFilterSensor::sense()) {
    return false;
  }
  
  float v = float(value());

  //DEBUG_PRINT(F("reading sensor value: ")); 
  //DEBUG_PRINTLN(v);
  
  if (v <= currentMin) {
    currentMin = int(ETAquick * v + (1.0 - ETAquick)*float(currentMin));
  } else {
    currentMin = int(ETAslow * v + (1.0 - ETAslow)*float(currentMin));
  }
  
  if (v >= currentMax) {
    currentMax = int(ETAquick * v + (1.0 - ETAquick)*float(currentMax));
  } else {
    currentMax = int(ETAslow * v + (1.0 - ETAslow)*float(currentMax));
  }
  
  if (currentMin < 1) {
    currentMin = 1;
  }
  if (currentMax < 1) {
    currentMax = 1;
  }
  return true;
}



