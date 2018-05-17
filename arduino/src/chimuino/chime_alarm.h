#ifndef CHIME_ALARM_H
#define CHIME_ALARM_H

#include "chime_bluetooth.h"

#include "RTClib.h"

class ChimeAlarm: public BluetoothCommandListener {
  
  private:

    // user settings
    bool enabled = false;                                           
    unsigned short hour = 7;                               // the hour 
    unsigned short minutes = 15;                           // minutes
    unsigned short durationSoft = 15;    
    unsigned short durationLong = 15;
    bool done = false;                          // true if we did the job of waking up the user

    bool monday = true;
    bool tuesday = true;
    bool wednesday = true;
    bool thursday = true;
    bool friday = true;
    bool saterday = true;
    bool sunday = true;
    
    // other
    const char* name;                                 // the name of the alarm (like ALARM1, ALARM2...)
    unsigned short name_length;                 // the length of the name
    
  public:
    // constructor
    ChimeAlarm(const char* name);
    
    // prints the state of the alarm in the serial 
    void debugSerial();

    bool shouldPrering(DateTime now);
    bool shouldRing(DateTime now);
    
    // inherited
    virtual BluetoothListenerAnswer processBluetoothGet(char* str, SoftwareSerial* BTSerial);
    virtual BluetoothListenerAnswer processBluetoothSet(char* str, SoftwareSerial* BTSerial);
    virtual BluetoothListenerAnswer processBluetoothDo(char* str, SoftwareSerial* BTSerial);


};

#endif // CHIME_ALARM_H





