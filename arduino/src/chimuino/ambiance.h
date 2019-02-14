#ifndef AMBIANCE_H
#define AMBIANCE_H

#include "chime.h"
#include "chime_bluetooth.h"
#include "chime_lightsensor.h"
#include "chime_soundsensor.h"
#include "persist.h"

class Ambiance: public BluetoothInformationProducer,
                public IntentionProvider {
  
  private:
    bool enabled = true;             // play sound from time to time 
    ChimeSoundSensor* soundSensor;
    ChimeLightSensor* lightSensor;
    Persist* persist;

    void storeState();

    virtual void publishBluetoothData();

  public:
    // constructor
    Ambiance();
    void setup(Persist* _persist, ChimeSoundSensor* soundSensor, ChimeLightSensor* lightSensor);
    
    void debugSerial();

    bool isEnabled() { return enabled; }
    
    // inherited
    virtual BluetoothListenerAnswer receivedAmbiance(ble_ambiance content);
    virtual Intention proposeNextMode(enum mode current_mode, unsigned long next_planned_action);


};

#endif // AMBIANCE_H

