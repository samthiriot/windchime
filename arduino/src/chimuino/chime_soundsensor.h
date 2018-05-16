
#ifndef CHIME_SOUNDSENSOR_H
#define CHIME_SOUNDSENSOR_H

// in milliseconds
#define CHIME_SOUNDSENSOR_MEASURES_FREQUENCY 100 

#define CHIME_SOUNDSENSOR_MEASURES_ETA 0.5 

#include "sensors_filtering.h"

class ChimeSoundSensor {
  
  private:
  
    // hardware config
    char pin;

    // 
    LowPassFilterSensor sensor;                                                           // the sensor plugged on the pin
      
    // state
    unsigned short soundLevelMin = 0;                                                    // the lowest sound we ever measured
    unsigned short soundLevelMax = 1024;                                                 // the higest ever heard
    unsigned short quietThreshold = 20;                  // the silence / noise threshold
    unsigned short soundLastQuiet = 0;                                                   // the last time it was quiet 
    unsigned short soundLastNotQuiet = 0;                                                // the last time it was noisy

    // measures the current level of sound
    unsigned short measureSoundLevel();
    
  public:
    
    ChimeSoundSensor(const char pin);
    
    void setup();
    
    // reads sound level from the environment
    void perceive();
    
    // prints debug information in the Serial port
    void debugSerial();

    // returns a filtered and averaged sound level
    unsigned short getSoundLevel();

    // returns true if it is quiet now
    bool isQuiet();
    
};


#endif // CHIME_SOUNDSENSOR_H






