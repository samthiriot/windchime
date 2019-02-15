              
// +---------------------------------------------+
// |      CLOCK                                  |
// |          precise time and alarms            |
// +---------------------------------------------+

#ifndef CHIME_CLOCK_H
#define CHIME_CLOCK_H

#include "bluetooth.h"

// instanciates an RTC object
#include <TimeLib.h>
#include <DS3232RTC.h>

// TODO share uptime

// TODO set our alarms as RTC alarms
// TODO use IRQs to wake up the Arduino

class ChimeClock: public BluetoothInformationProducer {
  
  private:

    virtual void publishBluetoothData();

  public:
    ChimeClock();
    
    void setup();

    void perceive();
    
    // prints debug information in the Serial port
    void debugSerial();

    unsigned long startTimestamp;
    
    // return current datetime
    time_t now(); 

    // set alarms
    // TODO void setAlarm1(unsigned short hour, unsigned short minutes);
    // TODO void setAlarm2(unsigned short hour, unsigned short minutes);

    // reads and return a temperature in Celcius
    float getTemperature();
    
    // inherited
    virtual BluetoothListenerAnswer receivedCurrentDateTime(ble_datetime content);

};


#endif // CHIME_CLOCK_H
