              
// +---------------------------------------------+
// |      CLOCK                                  |
// |          precise time and alarms            |
// +---------------------------------------------+

#ifndef CHIME_CLOCK_H
#define CHIME_CLOCK_H

#include "chime_bluetooth.h"

// instanciates an RTC object
#include <TimeLib.h>
#include <DS3232RTC.h>

class ChimeClock: public BluetoothCommandListener {
  
  private:

  public:
    ChimeClock();
    
    void setup();
    
    // prints debug information in the Serial port
    void debugSerial();
    
    // return current datetime
    time_t now(); 

    // set alarms
    // TODO void setAlarm1(unsigned short hour, unsigned short minutes);
    // TODO void setAlarm2(unsigned short hour, unsigned short minutes);

    // reads and return a temperature in Celcius
    float getTemperature();
    
    // inherited
    virtual BluetoothListenerAnswer processBluetoothGet(char* str, SoftwareSerial* BTSerial);
    virtual BluetoothListenerAnswer processBluetoothSet(char* str, SoftwareSerial* BTSerial);
    virtual BluetoothListenerAnswer processBluetoothDo(char* str,  SoftwareSerial* BTSerial);

};


#endif // CHIME_CLOCK_H






