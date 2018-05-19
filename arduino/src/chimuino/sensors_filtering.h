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
    void setup();
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
  
    float ETAquick = 0.95;
    float ETAslow = 0.000001;
  
    float currentMin;
    float currentMax;
    
  public:
  
    LowPassFilterSensorWithMinMax(const float ETA, const byte pin, const unsigned int period, 
                                  const float ETAslow, const float ETAquick
                                  );
    void setup(int initialMin = -1, int initialMax = -1);
    bool sense();
    
    int envelopeMin() { return int(currentMin); }
    int envelopeMax() { return int(currentMax); }
    bool isMeaningful() { return int(currentMax - currentMin) > 10; }
};


#endif // SENSORS_FILTERING_H


