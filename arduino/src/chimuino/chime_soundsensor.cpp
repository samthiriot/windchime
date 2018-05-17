#include "chime_soundsensor.h"

#include <Arduino.h>
#include <Streaming.h>

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
  DEBUG_PRINT(F("SOUND LEVEL: ")); DEBUG_PRINT(sensor.value()); 
  DEBUG_PRINTLN(isQuiet() ? F("(quiet)"):F("(noisy)"));
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


BluetoothListenerAnswer ChimeSoundSensor::processBluetoothGet(char* str, SoftwareSerial* BTSerial) {

  if (strncmp(str, "SOUNDLEVEL", 10) == 0) {
    
    *BTSerial << F("SOUNDLEVEL IS ") 
              << _DEC(getSoundLevel()) << ' ' 
              << (isQuiet() ? F("QUIET"): F("NOISY"))
              << endl;
              
    return SUCCESS;
  } 

  if (strncmp(str, "SOUNDTHRESHOLD", 14) == 0) {
    
    *BTSerial << F("SOUNDTHRESHOLD IS ")
              << _DEC(quietThreshold) 
              << endl;
              
    return SUCCESS;
  } 
    
  return NOT_CONCERNED;
}



BluetoothListenerAnswer ChimeSoundSensor::processBluetoothSet(char* str, SoftwareSerial* BTSerial) {
  // TODO threshold?
  return NOT_CONCERNED;
}

BluetoothListenerAnswer ChimeSoundSensor::processBluetoothDo(char* str, SoftwareSerial* BTSerial) {
  return NOT_CONCERNED;
}




