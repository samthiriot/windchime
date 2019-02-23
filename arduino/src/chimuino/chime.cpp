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
    case AMBIANCE_TINTEMENT:  return ("ambiance (light)");
    case AMBIANCE_PREREVEIL:  return ("ambiance (medium)");
    case AMBIANCE_REVEIL:     return ("ambiance (strong)");
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

void Chime::doDemoLight() {
  demoAsked = DEMO_LIGHT;
}
void Chime::doDemoMedium() {
  demoAsked = DEMO_MEDIUM;
}
void Chime::doDemoStrong() {
  demoAsked = DEMO_STRONG;
}

Intention Chime::proposeNextMode(Intention currentIntention) {

  if (demoAsked != NOTHING) {
    enum mode demoAsked2 = demoAsked;
    demoAsked = NOTHING;
    return Intention { demoAsked2,  millis() }; 
    /*switch (currentIntention.what) {
      case PREALARM1:
      case PREALARM2:
      case ALARM1:
      case ALARM2:
        // there is something more serious ongoing; let's do it
        
        default;
       case 
    }*/
  }
  
  return currentIntention;
  
}
