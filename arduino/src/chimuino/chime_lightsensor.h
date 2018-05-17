
#ifndef CHIME_LIGHTSENSOR_H
#define CHIME_LIGHTSENSOR_H

// in milliseconds
#define CHIME_LIGHTSENSOR_MEASURES_FREQUENCY 100 

#define CHIME_LIGHTSENSOR_MEASURES_ETA 0.5 

// default light threshold
#define PHOTOCELL_NIGHT_THRESHOLD 550                                     // TODO detect threshold

#include "sensors_filtering.h"

#include "chime_bluetooth.h"

class ChimeLightSensor: public BluetoothCommandListener {
  
  private:
  
    // hardware config
    char pin;

    // 
    LowPassFilterSensor sensor;                                                           // the sensor plugged on the pin

    unsigned short darkThreshold = PHOTOCELL_NIGHT_THRESHOLD;          // the default threshold
    
  public:
    
    ChimeLightSensor(const char pin);
    
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






