#ifndef CHIME_ALARM_H
#define CHIME_ALARM_H

#include "chime.h"
#include "chime_bluetooth.h"
#include "persist.h"

// TODO alarm with interrupts?

class ChimeAlarm: public BluetoothUser,
                  public IntentionProvider {
  
  private:

    // user settings
    bool enabled = false;                                           
    unsigned short start_hour = 7;                               // the hour 
    unsigned short start_minutes = 15;                           // minutes
    unsigned short durationSoft = 15;    
    unsigned short durationStrong = 15;
    // TODO bool done = false;                          // true if we did the job of waking up the user

    bool monday = true;
    bool tuesday = true;
    bool wednesday = true;
    bool thursday = true;
    bool friday = true;
    bool saterday = false;
    bool sunday = false;
    
    // other
    byte id;                                    // the id of the name of the alarm (like ALARM1, ALARM2...)    
    bool rightWeekdayForRing();                 // returns true if the weekday is compliant with our settings
    Persist* persist;

    void storeState();
    virtual void publishBluetoothData();
    
    // adapts the current alarm with data coming from bluetooth
    void updateAlarmWithData(ble_alarm content);
    
  public:
    // constructor
    ChimeAlarm(byte id);
    void setup(Persist* persist);
    
    // prints the state of the alarm in the serial 
    void debugSerial();

    bool shouldPrering();
    bool shouldRing();
    
    // inherited
    virtual BluetoothListenerAnswer receivedAlarm1(ble_alarm content);
    virtual BluetoothListenerAnswer receivedAlarm2(ble_alarm content);

    virtual Intention proposeNextMode(enum mode current_mode, unsigned long next_planned_action);


};

#endif // CHIME_ALARM_H





