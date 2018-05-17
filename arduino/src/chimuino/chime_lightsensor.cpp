#include "chime_lightsensor.h"

#include <Arduino.h>
#include <Streaming.h>

#include "debug.h"

ChimeLightSensor::ChimeLightSensor(const char _pin):
              sensor( CHIME_LIGHTSENSOR_MEASURES_ETA, _pin, 
                      CHIME_LIGHTSENSOR_MEASURES_FREQUENCY,// read every 100ms
                      CHIME_LIGHTSENSOR_ENVELOPE_ETA_SLOW, CHIME_LIGHTSENSOR_ENVELOPE_ETA_QUICK
                      )  
              {

  // save params
  pin = _pin;

}

void ChimeLightSensor::perceive() {
  sensor.sense();
}

void ChimeLightSensor::debugSerial() {
  DEBUG_PRINT(F("LIGHT LEVEL: ")); DEBUG_PRINT(sensor.value()); 
  DEBUG_PRINTLN(isDark() ? F("(dark)"):F("(lid)"));
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




BluetoothListenerAnswer ChimeLightSensor::processBluetoothGet(char* str, SoftwareSerial* BTSerial) {

  if (strncmp_P(str, PSTR("LIGHTLEVEL"), 10) == 0) {
    
    *BTSerial << F("LIGHTLEVEL IS ") 
              << _DEC(getLightLevel()) << ' ' 
              << (isDark() ? F("DARK"): F("LIT"))
              << endl;
              
    return SUCCESS;
  } 

  if (strncmp_P(str, PSTR("LIGHTTHRESHOLD"), 14) == 0) {
    
    *BTSerial << F("LIGHTTHRESHOLD IS ")
              << _DEC(darkThreshold) 
              << F(" [") << _DEC(sensor.envelopeMin()) << ':' << _DEC(sensor.envelopeMax()) << ']'
              << endl;
              
    return SUCCESS;
  } 
    
  return NOT_CONCERNED;
}



BluetoothListenerAnswer ChimeLightSensor::processBluetoothSet(char* str, SoftwareSerial* BTSerial) {
  return NOT_CONCERNED;
}

BluetoothListenerAnswer ChimeLightSensor::processBluetoothDo(char* str, SoftwareSerial* BTSerial) {
  return NOT_CONCERNED;
}


