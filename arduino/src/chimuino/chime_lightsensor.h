
#ifndef CHIME_LIGHTSENSOR_H
#define CHIME_LIGHTSENSOR_H

// in milliseconds
#define CHIME_LIGHTSENSOR_MEASURES_FREQUENCY 1000 
#define CHIME_LIGHTSENSOR_MEASURES_ETA 0.5 

#define CHIME_LIGHTSENSOR_ENVELOPE_ETA_SLOW 0.000001
#define CHIME_LIGHTSENSOR_ENVELOPE_ETA_QUICK 0.999

// default light threshold
#define PHOTOCELL_NIGHT_THRESHOLD 30                                     // TODO detect threshold

#include "sensors_filtering.h"

#include "chime_bluetooth.h"

#include <Arduino.h>


class ChimeLightSensor: public BluetoothCommandListener {
  
  private:
  
    // hardware config
    byte pin;

    // 
    LowPassFilterSensorWithMinMax sensor;                                                           // the sensor plugged on the pin

    unsigned short darkThreshold = PHOTOCELL_NIGHT_THRESHOLD;          // the default threshold
    
    byte factorThreshold = 50;                // the quiet / noisy threshold in % will be defined as min+(max-min)*factor; so a factor of 0 will always lead to noisy, a factor of 1 will alwats lead to quiet

  public:
    
    ChimeLightSensor(const byte pin);
    
    void setup();
    
    // reads light level from the environment
    void perceive();
    
    // prints debug information in the Serial port
    void debugSerial();

    // returns a filtered and averaged sound level
    unsigned short getLightLevel();

    // returns true if it is dark now
    bool isDark();

    // inherited
    virtual BluetoothListenerAnswer processBluetoothGet(char* str, SoftwareSerial* BTSerial);
    virtual BluetoothListenerAnswer processBluetoothSet(char* str, SoftwareSerial* BTSerial);
    virtual BluetoothListenerAnswer processBluetoothDo(char* str, SoftwareSerial* BTSerial);

};


#endif // CHIME_LIGHTSENSOR_H






