#include "chime_soundsensor.h"

#include <Arduino.h>

#include "debug.h"

ChimeSoundSensor::ChimeSoundSensor(const char _pin):
              sensor(CHIME_SOUNDSENSOR_MEASURES_ETA, _pin, CHIME_SOUNDSENSOR_MEASURES_FREQUENCY)  // read every 100ms 
              {

  // save params
  pin = _pin;

  
}

void ChimeSoundSensor::perceive() {
  sensor.sense();         // update the measure of sound
  
}

void ChimeSoundSensor::debugSerial() {
  DEBUG_PRINT("SOUND LEVEL: "); DEBUG_PRINT(sensor.value()); 
  DEBUG_PRINTLN(isQuiet() ? "(quiet)":"(noisy)");
}

void ChimeSoundSensor::setup() {
  
  pinMode(pin, INPUT); 
        
  soundLastQuiet = soundLastNotQuiet  = 0;                        // let's say the last time it was quiet and noisy was far ago
  soundLevelMin = soundLevelMax = analogRead(pin);                // init the sound levels known so far
}

unsigned short ChimeSoundSensor::measureSoundLevel() {
  return sensor.value();
}

unsigned short ChimeSoundSensor::getSoundLevel() {
  unsigned short raw = measureSoundLevel();
  
}

bool ChimeSoundSensor::isQuiet() {
  return getSoundLevel() <= quietThreshold;
}






