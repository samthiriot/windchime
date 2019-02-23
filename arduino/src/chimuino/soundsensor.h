
#ifndef CHIME_SOUNDSENSOR_H
#define CHIME_SOUNDSENSOR_H

// in milliseconds
#define CHIME_SOUNDSENSOR_MEASURES_FREQUENCY 100 
#define CHIME_SOUNDSENSOR_MEASURES_ETA 100.0

#define CHIME_SOUNDSENSOR_ENVELOPE_ETA_SLOW 0.0000001 // that's the max precision we can have...
#define CHIME_SOUNDSENSOR_ENVELOPE_ETA_QUICK 999.0

#include "filtering.h"
#include "bluetooth.h"
#include "persist.h"

#include <Arduino.h>

/**
 * Implements a low pass filter sensor with min and max
 * adapted to the specific case of sound: we do not measure a single point,
 * but rather the variations received from the sensor during several milliseconds.
 */
class SoundLowPassFilterSensorWithMinMax: public LowPassFilterSensorWithMinMax {
  
  private:
  
    
  public:
  
    SoundLowPassFilterSensorWithMinMax(const float ETA, const byte pin, const unsigned int period, 
                                  const float ETAslow, const float ETAquick
                                  );
                                  
    bool sense();
    
};

class ChimeSoundSensor: public BluetoothInformationProducer {
  
  private:
  
    // hardware config
    byte pin;

    // 
    SoundLowPassFilterSensorWithMinMax sensor;                                                // the sensor plugged on the pin
      
    // state
    uint16_t lastLevelSentBluetooth = 0;
    unsigned short quietThreshold = 20;                  // the silence / noise threshold

    byte factorThreshold = 50;                // the quiet / noisy threshold will be defined as min+(max-min)*factor; so a factor of 0 will always lead to noisy, a factor of 1 will alwats lead to quiet
    
    // measures the current level of sound
    unsigned short measureSoundLevel();

    Persist* persist;

    void storeState();
    
    void publishBluetoothDataSettings();
    void publishBluetoothDataSensor();
    virtual void publishBluetoothData();

  public:
    
    ChimeSoundSensor(const byte pin);
    
    void setup(Persist* persist);
    
    // reads sound level from the environment
    void perceive();
    
    // prints debug information in the Serial port
    void debugSerial();

    // returns a filtered and averaged sound level
    unsigned short getSoundLevel();

    // returns true if it is quiet now
    bool isQuiet();
    
    // inherited
    virtual BluetoothListenerAnswer receivedSoundSettings(ble_sound_settings content);

};


#endif // CHIME_SOUNDSENSOR_H
