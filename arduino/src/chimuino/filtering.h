#ifndef SENSORS_FILTERING_H
#define SENSORS_FILTERING_H

#include <Arduino.h>

/**
 * reads data from the arduino pin <pin> every <period>ms, 
 * and smooth the value according to <ETA>. call sense() to update value, 
 * and value() to get the value.
 * AKA  simple linear recursive exponential filter
 */
class LowPassFilterSensor {
  
  protected:

    float ETA;                  // the smoothing constant in 0:1. 0.5= mean, 0.9 strong weight on last value, 0.1 smooth a lot
    byte pin;                   // the pin to read data from
    uint16_t period;        // how often to listen for the value (in milliseconds)

    unsigned long lastReading = 0;    // timestamp of the last reading
    uint8_t pastvalue;
    uint8_t update_value(uint8_t v, uint8_t pastvalue, float ETA);
    uint8_t update_value(float vf, uint8_t pastvalue, float ETA);

  public:
   
    // constructor
    LowPassFilterSensor(const float ETA, const byte pin, const uint16_t period);
    void setup();
    /**
     * Returns true if something was sensed, false if it was not yet time
     */
    bool sense();
    
    uint8_t value() { return pastvalue; }
};


/**
 * a sensor with low pass filter, 
 * which keeps smoothed min and max.
 */
class LowPassFilterSensorWithMinMax: public LowPassFilterSensor {
  
  protected:
  
    float ETAquick = 0.95;
    float ETAslow = 0.000001;
  
    uint8_t currentMin;
    uint8_t currentMax;

    void adaptMinMax(float v);
    
  public:
  
    LowPassFilterSensorWithMinMax(const float ETA, const byte pin, const unsigned int period, 
                                  const float ETAslow, const float ETAquick
                                  );
    void setup(uint8_t initialMin = -1, uint8_t initialMax = -1);
    bool sense();
    
    uint8_t envelopeMin() { return currentMin; }
    uint8_t envelopeMax() { return currentMax; }
    bool isMeaningful() { return uint8_t(currentMax - currentMin) > 10; }
};


#endif // SENSORS_FILTERING_H
