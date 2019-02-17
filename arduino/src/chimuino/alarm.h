#ifndef CHIME_ALARM_H
#define CHIME_ALARM_H

#include "chime.h"
#include "bluetooth.h"
#include "persist.h"

// TODO alarm with interrupts?

const char message_alarm [] PROGMEM = "ALARM";


class ChimeAlarm: public BluetoothInformationProducer,
                  public IntentionProvider {
  
  private:

    // user settings
    bool enabled = false;                                           
    uint8_t start_hour = 7;                               // the hour 
    uint8_t start_minutes = 15;                           // minutes
    uint8_t durationSoft = 15;    
    uint8_t durationStrong = 15;
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
    

  public:
    // constructor
    ChimeAlarm(byte id);
    void setup(Persist* persist);
    
    // prints the state of the alarm in the serial 
    void debugSerial();

    bool shouldPrering();
    bool shouldRing();

    // adapts the current alarm with data coming from bluetooth
    void updateAlarmWithData(ble_alarm content);
    
    // inherited
    
    virtual Intention proposeNextMode(Intention currentIntention);


};

#endif // CHIME_ALARM_H
