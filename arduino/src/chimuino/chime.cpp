#include "chime.h"

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
    default:                  return ("?");
  };
};

Chime::Chime() {
  
}

BluetoothListenerAnswer Chime::processBluetoothGet(char* str, SoftwareSerial* BTSerial) {
  
}

BluetoothListenerAnswer Chime::processBluetoothSet(char* str, SoftwareSerial* BTSerial) {
  
}
    
BluetoothListenerAnswer Chime::processBluetoothDo(char* str, SoftwareSerial* BTSerial) {
  
}


