// +---------------------------------------------+
// |      STEPPER                                |
// |          move the chime                     |
// +---------------------------------------------+


#ifndef CHIME_STEPPER_H
#define CHIME_STEPPER_H


#include <Stepper.h>

#define MOTOR_SPEED_SLOW 5                                        // speed silencious
#define MOTOR_SPEED_QUICK 10                                      // speed quick but less silencious
#define MOTOR_SPEED_MAX 15                                        // max speed never to go beyond

class ChimeStepper {

  private:
    // initial settings
    unsigned short steps;
    unsigned short pin1, pin2, pin3, pin4;
 
    // state
    unsigned short pull_light, pull_medium, pull_strong;
    
    Stepper motor; 

  public:
    // constructor
    ChimeStepper(
      // count of steps for this engine
      unsigned short steps, 
      // pins on which the engine is connected to
      unsigned short pin1, unsigned short pin2, unsigned short pin3, unsigned short pin4);
    
    void setup();
    // relax the engine so we don't consume energy nor emit noise
    void doFreeWheel();
    void doReveil();
    void doPreReveil();
    void doTintement();
    
};

#endif // CHIME_STEPPER_H






