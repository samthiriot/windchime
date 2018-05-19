#include "chime_lightsensor.h"

#include <Arduino.h>
#include <Streaming.h>

#include "debug.h"

ChimeLightSensor::ChimeLightSensor(const byte _pin):
              sensor( CHIME_LIGHTSENSOR_MEASURES_ETA, _pin, 
                      CHIME_LIGHTSENSOR_MEASURES_FREQUENCY,// read every 100ms
                      CHIME_LIGHTSENSOR_ENVELOPE_ETA_SLOW, CHIME_LIGHTSENSOR_ENVELOPE_ETA_QUICK
                      )  
              {

  // save params
  pin = _pin;

}

void ChimeLightSensor::perceive() {
  
  if (sensor.sense()) {
    // the sensor was updated

    // ... update the threshold!
    darkThreshold = sensor.envelopeMin() + int(float(factorThreshold) * float(sensor.envelopeMax() - sensor.envelopeMin()) / 100.);
  
  }
}

void ChimeLightSensor::debugSerial() {

  #ifdef DEBUG
  Serial << F("LIGHTLEVEL IS ") 
         << _DEC(getLightLevel()) << ' ' 
         << (isDark() ? F("DARK"): F("LIT")) << ','
         << _DEC(factorThreshold) << '>'
         << _DEC(darkThreshold) << F(" in ")
         << '[' << _DEC(sensor.envelopeMin()) << ':' << _DEC(sensor.envelopeMax()) << ']'
         << endl;
  #endif
}

void ChimeLightSensor::setup() {

  DEBUG_PRINTLN(F("init: light sensor..."));

  sensor.setup(0, -1); // we assume no light means 0, but have no idea of the max
  sensor.sense();

  DEBUG_PRINTLN(F("init: light sensor ok"));
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
              << _DEC(factorThreshold) << ' '
              << _DEC(darkThreshold) 
              << F(" [") << _DEC(sensor.envelopeMin()) << ':' << _DEC(sensor.envelopeMax()) << ']'
              << endl;
              
    return SUCCESS;
  } 
    
  return NOT_CONCERNED;
}



BluetoothListenerAnswer ChimeLightSensor::processBluetoothSet(char* str, SoftwareSerial* BTSerial) {
  
  if (strncmp_P(str, PSTR("LIGHTTHRESHOLD "), 15) == 0) {

    DEBUG_PRINT(F("received light threshold: ")); DEBUG_PRINTLN(str);

    sscanf(
          str + 15,           // skip the command
          "%u",               // expected format 
          &factorThreshold    // store directly in our variables
          );                

    if (factorThreshold < 0) { factorThreshold = 0; }
    else if (factorThreshold > 100) { factorThreshold = 100; }
    
    *BTSerial << F("LIGHTTHRESHOLD SET") << endl;
   
    debugSerial();

    return SUCCESS;
  }
  
  return NOT_CONCERNED;
}

BluetoothListenerAnswer ChimeLightSensor::processBluetoothDo(char* str, SoftwareSerial* BTSerial) {
  return NOT_CONCERNED;
}


