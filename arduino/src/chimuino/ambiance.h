#ifndef AMBIANCE_H
#define AMBIANCE_H

#include "chime_bluetooth.h"


class Ambiance: public BluetoothCommandListener {
  
  private:
    bool enabled = true;             // play sound from time to time 
  
  public:
    // constructor
    Ambiance();
    void setup();
    
    void debugSerial();

    bool isEnabled() { return enabled; }
    
    // inherited
    virtual BluetoothListenerAnswer processBluetoothGet(char* str, SoftwareSerial* BTSerial);
    virtual BluetoothListenerAnswer processBluetoothSet(char* str, SoftwareSerial* BTSerial);
    virtual BluetoothListenerAnswer processBluetoothDo(char* str, SoftwareSerial* BTSerial);


};

#endif // AMBIANCE_H

