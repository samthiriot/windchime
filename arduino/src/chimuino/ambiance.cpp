
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
    TRACE_PRINTLN(PGMSTR(msg_loaded_saved_state));
  } else {
    ERROR_PRINTLN(PGMSTR(msg_not_persisted_default_state));
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
         << (enabled ? PGMSTR(msg_enabled) : PGMSTR(msg_disabled) )
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

Intention Ambiance::proposeNextMode(Intention currentIntention) {

    bool canPlay = isEnabled() and !lightSensor->isDark() and soundSensor->isQuiet();
    
   if (currentIntention.what == NOTHING and canPlay) {

    unsigned long additionalDelay;
    
    int r = random(0,100);
    if (r <= 60) { 
      additionalDelay = millis() + random(10,60)*1000l;
      TRACE_PRINT(F("silence for ")); TRACE_PRINT(additionalDelay/1000); TRACE_PRINTLN('s');
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
          (currentIntention.what == AMBIANCE_TINTEMENT or currentIntention.what == AMBIANCE_PREREVEIL or currentIntention.what == AMBIANCE_REVEIL)
          ) {
      // disable planned ambiance if any!
      TRACE_PRINTLN(F("no more sound for ambiance."));
      return Intention { NOTHING,  millis() };
  }
  
  return currentIntention;
  
}
