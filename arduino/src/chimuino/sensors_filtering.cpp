
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

void LowPassFilterSensor::sense() {
  long now = millis();
  if (now - lastReading > period) {
    // let's read
    pastvalue = ETA*analogRead(pin) + (1-ETA)*pastvalue;
    lastReading = now;
  }
}

int LowPassFilterSensor::value() {
  return lastReading;
}






