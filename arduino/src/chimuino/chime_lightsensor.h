
#ifndef CHIME_LIGHTSENSOR_H
#define CHIME_LIGHTSENSOR_H

// in milliseconds
#define CHIME_LIGHTSENSOR_MEASURES_FREQUENCY 1000 
#define CHIME_LIGHTSENSOR_MEASURES_ETA 0.5 

#define CHIME_LIGHTSENSOR_ENVELOPE_ETA_SLOW 0.00000001
#define CHIME_LIGHTSENSOR_ENVELOPE_ETA_QUICK 0.999

// default light threshold
#define PHOTOCELL_NIGHT_THRESHOLD 30                                     // TODO detect threshold

#include "chime.h"
#include "sensors_filtering.h"

#include "chime_bluetooth.h"
#include "persist.h"

#include <Arduino.h>


class ChimeLightSensor: public BluetoothUser,
                        public IntentionProvider {
  
  private:
  
    // hardware config
    byte pin;

    // 
    LowPassFilterSensorWithMinMax sensor;                                                           // the sensor plugged on the pin

    unsigned short darkThreshold = PHOTOCELL_NIGHT_THRESHOLD;          // the default threshold
    
    byte factorThreshold = 50;                // the quiet / noisy threshold in % will be defined as min+(max-min)*factor; so a factor of 0 will always lead to noisy, a factor of 1 will alwats lead to quiet
    bool previousIsDark = false;
    void storeState();

    Persist* persist;

    virtual void publishBluetoothData();

  public:
    
    ChimeLightSensor(const byte pin);
    
    void setup(Persist* persist);
    
    // reads light level from the environment
    void perceive();
    
    // prints debug information in the Serial port
    void debugSerial();

    // returns a filtered and averaged sound level
    unsigned short getLightLevel();

    // returns true if it is dark now
    bool isDark();

    // inherited
    // TODO receive level and/or adjustement of level demand
    virtual BluetoothListenerAnswer receivedLightSettings(ble_light_settings content);
    virtual Intention proposeNextMode(enum mode current_mode, unsigned long next_planned_action);

};


#endif // CHIME_LIGHTSENSOR_H






