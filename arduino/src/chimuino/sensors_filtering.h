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

    float ETA;                  // the smoothing constant. 0.5 = mean, 0.9 strong weight on last value, 0.1 smooth peaks
    char pin;                   // the pin to read data from
    unsigned int period;        // how often to listen for the value

    unsigned long lastReading = 0;    // timestamp of the last reading
    int pastvalue;
    
  public:
   
    // constructor
    LowPassFilterSensor(const float ETA, const char pin, const unsigned int period);
    void sense();
    int value();
    
};

#endif // SENSORS_FILTERING_H





