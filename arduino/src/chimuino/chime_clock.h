              
// +---------------------------------------------+
// |      CLOCK                                  |
// |          precise time and alarms            |
// +---------------------------------------------+

#ifndef CHIME_CLOCK_H
#define CHIME_CLOCK_H

#include "RTClib.h"
#include "chime_bluetooth.h"

class ChimeClock: public BluetoothCommandListener {
  
  private:
    RTC_DS3231 rtc;

  public:
    ChimeClock();
    
    void setup();
    
    // prints debug information in the Serial port
    void debugSerial();
    
    // return current datetime
    DateTime now(); 
    
    // inherited
    virtual BluetoothListenerAnswer processBluetoothGet(char* str, SoftwareSerial* BTSerial);
    virtual BluetoothListenerAnswer processBluetoothSet(char* str, SoftwareSerial* BTSerial);
    virtual BluetoothListenerAnswer processBluetoothDo(char* str,  SoftwareSerial* BTSerial);

};


#endif // CHIME_CLOCK_H






