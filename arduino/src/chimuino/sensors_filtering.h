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
  
  private:

    float ETA;                  // the smoothing constant in 0:100. 50 = mean, 90 strong weight on last value, 100 smooth a lot
    byte pin;                   // the pin to read data from
    unsigned int period;        // how often to listen for the value (in milliseconds)

    unsigned long lastReading = 0;    // timestamp of the last reading
    int pastvalue;
    
  public:
   
    // constructor
    LowPassFilterSensor(const float ETA, const byte pin, const unsigned int period);

    /**
     * Returns true if something was sensed, false if it was not yet time
     */
    bool sense();
    
    int value() { return pastvalue; }
};


/**
 * a sensor with low pass filter, 
 * which keeps smoothed min and max.
 */
class LowPassFilterSensorWithMinMax: public LowPassFilterSensor {
  
  private:
  
    float ETAquick = 90;
    float ETAslow = 10;
  
    int currentMin;
    int currentMax;
    
  public:
  
    LowPassFilterSensorWithMinMax(const float ETA, const byte pin, const unsigned int period, const float ETAquick, const float ETAslow);
    bool sense();
    
    int envelopeMin() { return currentMax; }
    int envelopeMax() { return currentMin; }
};


#endif // SENSORS_FILTERING_H


