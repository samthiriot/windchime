#ifndef CHIME_ALARM_H
#define CHIME_ALARM_H

#include "chime_bluetooth.h"

enum mode {
  
  NOTHING,      // doing nothing

  WELCOME_SUN,  
  
  PREALARM1,
  PREALARM2,
  
  ALARM1,       // being ringing alam1
  ALARM2,       //               alarm2
  
  SILENCE,      // do not play sound

  CALIBRATING,  // calibration of sound is ongoing
  
  AMBIANCE_TINTEMENT,    // hear a very light bell sound
  AMBIANCE_PREREVEIL,    // hear the bell, enough to be conscious of their existence
  AMBIANCE_REVEIL        // hear the bells so much it should awake you
};

char* mode2str(enum mode v);


class Chime: public BluetoothCommandListener {
  
  private:

    
  public:
    // constructor
    Chime();
    void setup();
    
    void debugSerial();
    
    // inherited
    virtual BluetoothListenerAnswer processBluetoothGet(char* str, SoftwareSerial* BTSerial);
    virtual BluetoothListenerAnswer processBluetoothSet(char* str, SoftwareSerial* BTSerial);
    virtual BluetoothListenerAnswer processBluetoothDo(char* str, SoftwareSerial* BTSerial);


};

#endif // CHIME_ALARM_H

