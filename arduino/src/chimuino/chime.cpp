#include "chime.h"

#include "debug.h"

#include <Arduino.h>
#include <Streaming.h>


char* mode2str(enum mode v) {
  switch (v) {
    case NOTHING:             return ("nothing");
    case WELCOME_SUN:         return ("welcome sun");
    case PREALARM1:           return ("prealarm 1");
    case PREALARM2:           return ("prealarm 2");
    case ALARM1:              return ("alarm 1");
    case ALARM2:              return ("alarm 2");
    case SILENCE:             return ("silence");
    case CALIBRATING:         return ("calibration");
    case AMBIANCE_TINTEMENT:  return ("mood: tintement");
    case AMBIANCE_PREREVEIL:  return ("mood: prereveil");
    case AMBIANCE_REVEIL:     return ("mood: reveil");
    case DEMO_LIGHT:          return ("demo light");
    case DEMO_MEDIUM:         return ("demo medium");
    case DEMO_STRONG:         return ("demo strong");
    default:                  return ("?");
  };
};

Chime::Chime() {
  
}

void Chime::setup(ChimeSoundSensor* _soundSensor, ChimeStepper* _stepper) {

  // save the references of sensor and effector
  soundSensor = _soundSensor;
  stepper = _stepper;
 
}

void Chime::publishBluetoothData() {

  // TODO 
  // publish FIRMWARE_VERSION
  // publish current action?
  
  // this->bluetooth->publishAmbiance(content);
}

BluetoothListenerAnswer Chime::receivedActionRing(ble_ring_action action) {

  // TODO react action !

  /*
  demoAsked = DEMO_LIGHT;
  demoAsked = DEMO_MEDIUM;
  demoAsked = DEMO_STRONG;
  */

  return PROCESSED;  
};

Intention Chime::proposeNextMode(enum mode current_mode, unsigned long next_planned_action) {

  if ( (current_mode == NOTHING or current_mode == SILENCE) and demoAsked != NOTHING) {
    TRACE_PRINT("Doing demo of "); TRACE_PRINTLN(mode2str(demoAsked));
    enum mode demoAsked2 = demoAsked;
    demoAsked = NOTHING;
    return Intention { demoAsked2,  millis() }; 
  }

  return Intention { current_mode, next_planned_action };
  
}
