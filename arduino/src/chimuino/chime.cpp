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

BluetoothListenerAnswer Chime::processBluetoothGet(char* str, SoftwareSerial* BTSerial) {
  
  if (strncmp_P(str, PSTR("VERSION"), 7) == 0) {

    *BTSerial << F("VERSION IS ") 
              << FIRMWARE_VERSION
              << endl;

    DEBUG_PRINTLN(F("SENT VERSION"));
     
    return SUCCESS;
  }

  return NOT_CONCERNED;
}

BluetoothListenerAnswer Chime::processBluetoothSet(char* str, SoftwareSerial* BTSerial) {
  return NOT_CONCERNED;
}
    
BluetoothListenerAnswer Chime::processBluetoothDo(char* str, SoftwareSerial* BTSerial) {

  if (strncmp_P(str, PSTR("CHIME "), 6) == 0) {

    if (strncmp_P(str+6, PSTR("LIGHT"), 5) == 0) {
      DEBUG_PRINTLN("Asked for demo light");
      demoAsked = DEMO_LIGHT;
    } else if (strncmp_P(str+6, PSTR("MEDIUM"), 6) == 0) {
      DEBUG_PRINTLN("Asked for demo medium");
      demoAsked = DEMO_MEDIUM;
    }  else if (strncmp_P(str+6, PSTR("STRONG"), 6) == 0) {
      DEBUG_PRINTLN("Asked for demo strong");
      demoAsked = DEMO_STRONG;
    }
    *BTSerial << F("DOING DEMO") 
              << endl;
              
    return SUCCESS;
  } 
  
  return NOT_CONCERNED;

}

Intention Chime::proposeNextMode(enum mode current_mode, unsigned long next_planned_action) {

  if (demoAsked != NOTHING) {
    DEBUG_PRINT("Doing demo of "); DEBUG_PRINTLN(mode2str(demoAsked));
    enum mode demoAsked2 = demoAsked;
    demoAsked = NOTHING;
    return Intention { demoAsked2,  millis() }; 
  }

  return Intention { current_mode, next_planned_action };
  
}
