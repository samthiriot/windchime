#ifndef SENSORS_FILTERING_H
#define SENSORS_FILTERING_H

/**
 * reads data from the arduino pin <pin> every <period>ms, 
 * and smooth the value according to <ETA>. call sense() to update value, 
 * and value() to get the value.
 * AKA  simple linear recursive exponential filter
 */
class LowPassFilterSensor {
  
  private:

    float ETA;                  // the smoothing constant. 0.5 = mean, 0.9 strong weight on last value, 0.1 smooth a lot
    char pin;                   // the pin to read data from
    unsigned int period;        // how often to listen for the value (in milliseconds)

    unsigned long lastReading = 0;    // timestamp of the last reading
    int pastvalue;
    
  public:
   
    // constructor
    LowPassFilterSensor(const float ETA, const char pin, const unsigned int period);

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
  
    float ETAquick = 0.9;
    float ETAslow = 0.1;
  
    int currentMin;
    int currentMax;
    
  public:
  
    LowPassFilterSensorWithMinMax(const float ETA, const char pin, const unsigned int period, const float ETAquick, const float ETAslow);
    bool sense();
    
    int envelopeMin() { return currentMin; }
    int envelopeMax() { return currentMax; }
};


#endif // SENSORS_FILTERING_H


