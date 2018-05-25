#ifndef CHIME_ALARM_H
#define CHIME_ALARM_H

#include "chime.h"
#include "chime_bluetooth.h"


class ChimeAlarm: public BluetoothCommandListener,
                  public IntentionProvider {
  
  private:

    // user settings
    bool enabled = false;                                           
    unsigned short start_hour = 7;                               // the hour 
    unsigned short start_minutes = 15;                           // minutes
    unsigned short durationSoft = 15;    
    unsigned short durationStrong = 15;
    bool done = false;                          // true if we did the job of waking up the user

    bool monday = true;
    bool tuesday = true;
    bool wednesday = true;
    bool thursday = true;
    bool friday = true;
    bool saterday = true;
    bool sunday = true;
    
    // other
    byte id;                                    // the id of the name of the alarm (like ALARM1, ALARM2...)
    unsigned short name_length;                 // the length of the name
    
    bool rightWeekdayForRing();                 // returns true if the weekday is compliant with our settings
    
  public:
    // constructor
    ChimeAlarm(byte id);
    void setup();
    
    // prints the state of the alarm in the serial 
    void debugSerial();

    bool shouldPrering();
    bool shouldRing();
    
    // inherited
    virtual BluetoothListenerAnswer processBluetoothGet(char* str, SoftwareSerial* BTSerial);
    virtual BluetoothListenerAnswer processBluetoothSet(char* str, SoftwareSerial* BTSerial);
    virtual BluetoothListenerAnswer processBluetoothDo(char* str, SoftwareSerial* BTSerial);
    virtual Intention proposeNextMode(enum mode current_mode, unsigned long next_planned_action);


};

#endif // CHIME_ALARM_H





