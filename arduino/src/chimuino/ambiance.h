#ifndef AMBIANCE_H
#define AMBIANCE_H

#include "chime.h"
#include "chime_bluetooth.h"
#include "chime_lightsensor.h"
#include "chime_soundsensor.h"

class Ambiance: public BluetoothCommandListener,
                public IntentionProvider {
  
  private:
    bool enabled = true;             // play sound from time to time 
    ChimeSoundSensor* soundSensor;
    ChimeLightSensor* lightSensor;
    
  public:
    // constructor
    Ambiance();
    void setup(ChimeSoundSensor* soundSensor, ChimeLightSensor* lightSensor);
    
    void debugSerial();

    bool isEnabled() { return enabled; }
    
    // inherited
    virtual BluetoothListenerAnswer processBluetoothGet(char* str, SoftwareSerial* BTSerial);
    virtual BluetoothListenerAnswer processBluetoothSet(char* str, SoftwareSerial* BTSerial);
    virtual BluetoothListenerAnswer processBluetoothDo(char* str, SoftwareSerial* BTSerial);
    virtual Intention proposeNextMode(enum mode current_mode, unsigned long next_planned_action);


};

#endif // AMBIANCE_H

