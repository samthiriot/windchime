#include "chime_soundsensor.h"

#include <Arduino.h>
#include <Streaming.h>

#include "debug.h"

ChimeSoundSensor::ChimeSoundSensor(const char _pin):
              sensor(CHIME_SOUNDSENSOR_MEASURES_ETA, _pin, 
                    CHIME_SOUNDSENSOR_MEASURES_FREQUENCY, // read every 100ms
                    CHIME_SOUNDSENSOR_ENVELOPE_ETA_SLOW, CHIME_SOUNDSENSOR_ENVELOPE_ETA_QUICK
                    )  
              {

  // save params
  pin = _pin;

}

void ChimeSoundSensor::perceive() {
  
  if (sensor.sense()) {         // update the measure of sound  
    
    // the sensor was updated

    // ... update the threshold!
    quietThreshold = sensor.envelopeMin() + (float(factorThreshold)/100.) * (sensor.envelopeMax() - sensor.envelopeMin());
  
    #ifdef DEBUG
    debugSerial();
    #endif
  }
  
}

void ChimeSoundSensor::debugSerial() {
  
  #ifdef DEBUG
  Serial << F("SOUND LEVEL: ") << _DEC(sensor.value()) << ' ' 
         << ( isQuiet() ? F("(quiet)"):F("(noisy)") ) << ' '
         << _DEC(factorThreshold) << '>'
         << _DEC(quietThreshold) << ' '
         << '[' << _DEC(sensor.envelopeMin()) << ':' << _DEC(sensor.envelopeMax()) << ']'
         << endl;
  #endif
 
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

  if (strncmp_P(str, PSTR("SOUNDLEVEL"), 10) == 0) {
    
    *BTSerial << F("SOUNDLEVEL IS ") 
              << _DEC(getSoundLevel()) << ' ' 
              << (isQuiet() ? F("QUIET"): F("NOISY"))
              << endl;
              
    return SUCCESS;
  } 

  if (strncmp_P(str, PSTR("SOUNDTHRESHOLD"), 14) == 0) {
 
    *BTSerial << F("SOUNDTHRESHOLD IS ")
              << _DEC(factorThreshold) << ' '
              << _DEC(quietThreshold) 
              << F(" [") << _DEC(sensor.envelopeMin()) << ':' << _DEC(sensor.envelopeMax()) << ']'
              << endl;
              
    return SUCCESS;
  } 
    
  return NOT_CONCERNED;
}



BluetoothListenerAnswer ChimeSoundSensor::processBluetoothSet(char* str, SoftwareSerial* BTSerial) {
 
  if (strncmp_P(str, PSTR("SOUNDTHRESHOLD "), 15) == 0) {

    DEBUG_PRINT(F("received sound threshold: ")); DEBUG_PRINTLN(str);

    sscanf(
          str + 15,           // skip the command
          "%u",               // expected format 
          &factorThreshold    // store directly in our variables
          );                

    if (factorThreshold < 0) { factorThreshold = 0; }
    else if (factorThreshold > 100) { factorThreshold = 100; }
    
    *BTSerial << F("SOUNDTHRESHOLD SET") << endl;
   
    debugSerial();

    return SUCCESS;
  }
  
  return NOT_CONCERNED;
}

BluetoothListenerAnswer ChimeSoundSensor::processBluetoothDo(char* str, SoftwareSerial* BTSerial) {
  return NOT_CONCERNED;
}




