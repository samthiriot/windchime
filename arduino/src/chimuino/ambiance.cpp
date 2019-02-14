
#include "ambiance.h"

#include <Streaming.h>

#include "debug.h"
#include "utils.h"

Ambiance::Ambiance(): 
          BluetoothInformationProducer(),
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
    TRACE_PRINTLN(F("loaded ambiance data from saved state"));
  } else {
    ERROR_PRINTLN(F("no saved state for ambiance, defining the default state..."));
    storeState();
  }

}

void Ambiance::storeState() {
  persist->storeAmbiance(enabled);
}

void Ambiance::publishBluetoothData() {
  ble_ambiance content;
  content.enabled = enabled;
  this->bluetooth->publishAmbiance(content);
}

void Ambiance::debugSerial() {
  #ifdef TRACE
  Serial << F("Ambiance: ") 
         << (enabled ? F("enabled") : F("disabled"))
         << endl;
  #endif 
}

BluetoothListenerAnswer Ambiance::receivedAmbiance(ble_ambiance content) {

  // update our state with the content
  enabled = content.enabled;

  // store this state 
  storeState();

  // debug
  debugSerial();

  return PROCESSED;
}

Intention Ambiance::proposeNextMode(enum mode current_mode, unsigned long next_planned_action) {

    bool canPlay = isEnabled() and !lightSensor->isDark() and soundSensor->isQuiet();
    
   if (current_mode == NOTHING and canPlay) {

    unsigned long additionalDelay;
    
    int r = random(0,100);
    if (r <= 60) { 
      additionalDelay = millis() + random(10,60)*1000l;
      TRACE_PRINT(F("a bit of silence for ")); TRACE_PRINT(additionalDelay/1000); TRACE_PRINTLN('s');
      return Intention { SILENCE,  millis() + additionalDelay};
    } else if (r <= 65) {
      additionalDelay = millis() + random(4*60,15*60)*1000l;
      TRACE_PRINT(F("mood strong in ")); TRACE_PRINT(additionalDelay/1000); TRACE_PRINTLN('s');
      return Intention { AMBIANCE_REVEIL,  millis() + additionalDelay};
    } else if (r <= 75) {
      additionalDelay = millis() + random(4*60,10*60)*1000l;  
      TRACE_PRINT(F("mood medium in ")); TRACE_PRINT(additionalDelay/1000); TRACE_PRINTLN('s');
      return Intention { AMBIANCE_PREREVEIL,  millis() + additionalDelay};
    } else {
      additionalDelay = millis() + random(4*60,10*60)*1000l;  
      TRACE_PRINT(F("mood light in ")); TRACE_PRINT(additionalDelay/1000); TRACE_PRINTLN('s');
      return Intention { AMBIANCE_TINTEMENT,  millis() + additionalDelay};
    }
  } else if (!canPlay and 
          (current_mode == AMBIANCE_TINTEMENT or current_mode == AMBIANCE_PREREVEIL or current_mode == AMBIANCE_REVEIL)
          ) {
      // disable planned ambiance if any!
      TRACE_PRINTLN(F("no more sound for ambiance."));
      return Intention { NOTHING,  millis() };
  }
  
  return Intention { current_mode, next_planned_action };
  
}


