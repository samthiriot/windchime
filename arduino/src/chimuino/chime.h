#ifndef CHIME_H
#define CHIME_H

#include "chime_bluetooth.h"
#include "chime_soundsensor.h"
#include "chime_stepper.h"

// ... current version of the firmware.
//     should be upgraded at update time
#define FIRMWARE_VERSION "alpha_2018_05_16"


enum mode {
  
  NOTHING,      // doing nothing

  WELCOME_SUN,  
  
  PREALARM1,
  PREALARM2,
  
  ALARM1,       // being ringing alam1
  ALARM2,       //               alarm2

  DEMO_LIGHT,
  DEMO_MEDIUM,
  DEMO_STRONG,
  
  SILENCE,      // do not play sound

  CALIBRATING,  // calibration of sound is ongoing
  
  AMBIANCE_TINTEMENT,    // hear a very light bell sound
  AMBIANCE_PREREVEIL,    // hear the bell, enough to be conscious of their existence
  AMBIANCE_REVEIL        // hear the bells so much it should awake you
};

struct Intention {
  enum mode what;
  unsigned long when;  
};

class IntentionProvider {
    public:
 
      // called when a GET <SOMETHING> was received
      // will receive as arguments the <SOMETHING> char*, 
      // and a pointer to serial port to use to answer
      virtual Intention proposeNextMode(enum mode current_mode, unsigned long next_planned_action);

};

char* mode2str(enum mode v);


class Chime: public BluetoothInformationProducer,
             public IntentionProvider {
  
  private:
    ChimeSoundSensor* soundSensor;
    ChimeStepper* stepper;
    enum mode demoAsked = NOTHING;

    virtual void publishBluetoothData();

  public:
    // constructor
    Chime();
    void setup(ChimeSoundSensor* soundSensor, ChimeStepper* stepper);
    
    void debugSerial();
    
    // inherited

    virtual BluetoothListenerAnswer receivedActionRing(ble_ring_action action);
    
    virtual Intention proposeNextMode(enum mode current_mode, unsigned long next_planned_action);


};

#endif // CHIME_H

