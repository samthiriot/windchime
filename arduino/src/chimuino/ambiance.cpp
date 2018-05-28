
#include "ambiance.h"

#include <Streaming.h>

#include "debug.h"
#include "utils.h"

Ambiance::Ambiance(): 
          BluetoothCommandListener(),
          IntentionProvider()
          {

}

void Ambiance::setup(Persist* _persist, ChimeSoundSensor* _soundSensor, ChimeLightSensor* _lightSensor) {
  persist = _persist;
  lightSensor = _lightSensor;
  soundSensor = _soundSensor;

  if (persist->hasDataStored()) {
    // there is data persisted ! Let's load it :-)
    enabled = persist->getAmbianceEnabled();
    DEBUG_PRINTLN(F("loaded ambiance data from saved state"));
  } else {
    DEBUG_PRINTLN(F("no saved state for ambiance, defining the default state..."));
    storeState();
  }

}

void Ambiance::storeState() {
  persist->storeAmbiance(enabled);
}

void Ambiance::debugSerial() {
  #ifdef DEBUG
  Serial << F("Ambiance: ") 
         << (enabled ? F("enabled") : F("disabled"))
         << endl;
  #endif 
}

BluetoothListenerAnswer Ambiance::processBluetoothGet(char* str, SoftwareSerial* BTSerial) {

  if (strncmp_P(str, PSTR("AMBIANCE"), 8) == 0) {
    *BTSerial << F("AMBIANCE IS ") 
              << (enabled?'1':'0')
              << endl;
    DEBUG_PRINTLN(F("SENT AMBIANCE"));
    return SUCCESS;
  }
  
  return NOT_CONCERNED;
}

BluetoothListenerAnswer Ambiance::processBluetoothSet(char* str, SoftwareSerial* BTSerial) {
  
  if (strncmp_P(str, PSTR("AMBIANCE "), 9) == 0) {

    DEBUG_PRINTLN(F("Ambiance changed by bluetooth"));

    char cEnabled;

    sscanf(str + 9,                                                  // decode received datetime
           "%c", 
           &cEnabled);    

    enabled = char2bool(cEnabled);
    storeState();
    debugSerial();

    return SUCCESS;
  }
  
  return NOT_CONCERNED;
}

BluetoothListenerAnswer Ambiance::processBluetoothDo(char* str, SoftwareSerial* BTSerial) {
  return NOT_CONCERNED;
}

Intention Ambiance::proposeNextMode(enum mode current_mode, unsigned long next_planned_action) {

    bool canPlay = isEnabled() and !lightSensor->isDark() and soundSensor->isQuiet();
    
   if (current_mode == NOTHING and canPlay) {

    unsigned long additionalDelay;
    
    int r = random(0,100);
    if (r <= 60) { 
      additionalDelay = millis() + random(10,60)*1000l;
      DEBUG_PRINT(F("a bit of silence for ")); DEBUG_PRINT(additionalDelay/1000); DEBUG_PRINTLN('s');
      return Intention { SILENCE,  millis() + additionalDelay};
    } else if (r <= 65) {
      additionalDelay = millis() + random(4*60,15*60)*1000l;
      DEBUG_PRINT(F("mood strong in ")); DEBUG_PRINT(additionalDelay/1000); DEBUG_PRINTLN('s');
      return Intention { AMBIANCE_REVEIL,  millis() + additionalDelay};
    } else if (r <= 75) {
      additionalDelay = millis() + random(4*60,10*60)*1000l;  
      DEBUG_PRINT(F("mood medium in ")); DEBUG_PRINT(additionalDelay/1000); DEBUG_PRINTLN('s');
      return Intention { AMBIANCE_PREREVEIL,  millis() + additionalDelay};
    } else {
      additionalDelay = millis() + random(4*60,10*60)*1000l;  
      DEBUG_PRINT(F("mood light in ")); DEBUG_PRINT(additionalDelay/1000); DEBUG_PRINTLN('s');
      return Intention { AMBIANCE_TINTEMENT,  millis() + additionalDelay};
    }
  } else if (!canPlay and 
          (current_mode == AMBIANCE_TINTEMENT or current_mode == AMBIANCE_PREREVEIL or current_mode == AMBIANCE_REVEIL)
          ) {
      // disable planned ambiance if any!
      DEBUG_PRINTLN(F("no more sound for ambiance."));
      return Intention { NOTHING,  millis() };
  }
  
  return Intention { current_mode, next_planned_action };
  
}


