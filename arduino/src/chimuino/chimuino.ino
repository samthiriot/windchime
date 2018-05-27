// about bluetooth
// docs
// - https://medium.com/@yostane/using-the-at-09-ble-module-with-the-arduino-3bc7d5cb0ac2

#include "debug.h"

#include "persist.h"
#include "chime.h"
#include "chime_bluetooth.h"
#include "chime_stepper.h"
#include "chime_clock.h"
#include "chime_alarm.h"
#include "chime_soundsensor.h"
#include "chime_lightsensor.h"
#include "ambiance.h"


// sketch settings

#define DEBUG_SERIAL true                                         // config debug

// frequency for displaying debug messages on the state
#define FREQUENCY_DEBUG 5000
long lastDisplayDebug = millis();


// hardware settings

#define BLUETOOTH_RX 13
#define BLUETOOTH_TX 12

#define MOTOR_TOTAL_STEPS 2048                                    // in 4 steps mode
#define STEPPER_PIN_1 8
#define STEPPER_PIN_2 9
#define STEPPER_PIN_3 10
#define STEPPER_PIN_4 11

#define SOUND_PIN A1
#define PHOTOCELL_PIN A0                                                  // port for the photovoltaic cell

#define RANDOM_PIN A2                                                     // refers to a pin unconnected supposed to catch white noise 

Persist persist;      // persistence object

ChimeStepper stepper(MOTOR_TOTAL_STEPS, STEPPER_PIN_1, STEPPER_PIN_2, STEPPER_PIN_3, STEPPER_PIN_4);

ChimeClock clock;

ChimeAlarm alarm1(1);
ChimeAlarm alarm2(2);

ChimeBluetooth bluetooth(BLUETOOTH_RX, BLUETOOTH_TX);

ChimeSoundSensor soundSensor(SOUND_PIN);
ChimeLightSensor lightSensor(PHOTOCELL_PIN);

Chime chime;

Ambiance ambiance;


// +---------------------------------------------+
// |      RANDOM                                 |
// |            alea jacta est                   |
// +---------------------------------------------+
void setupRandom() {
    pinMode(RANDOM_PIN, INPUT);                                           // this pin is used to init the random network generator
    int seed = analogRead(RANDOM_PIN);
  #ifdef DEBUG_SERIAL
    Serial.print("init: random seed is "); Serial.print(seed); Serial.println();
  #endif
    randomSeed(seed);
}


unsigned long next_planned_action = millis();

mode current_mode = NOTHING;


/**
 * Entry point, ran before anything else by the Arduino.
 */
void setup() {
  
  DEBUG_SETUP();

  setupRandom();

  persist.setup();
  soundSensor.setup(&persist);
  lightSensor.setup(&persist);
  stepper.setup();
  clock.setup();
  alarm1.setup(&persist);
  alarm2.setup(&persist);
  bluetooth.setup();
  chime.setup(&soundSensor, &stepper);
  ambiance.setup(&persist, &soundSensor, &lightSensor);

  // ... add the chain of listeners, which might react to bluetooth commands
  bluetooth.addCommandInterpreter(&alarm1);
  bluetooth.addCommandInterpreter(&alarm2);
  bluetooth.addCommandInterpreter(&clock);
  bluetooth.addCommandInterpreter(&lightSensor);
  bluetooth.addCommandInterpreter(&soundSensor);
  bluetooth.addCommandInterpreter(&chime);
  bluetooth.addCommandInterpreter(&ambiance);

  DEBUG_PRINTLN("init: end.");

}
char truc[] = "GET VERSION\n";
bool debugNow = false;
void loop() {

  debugNow = millis() - lastDisplayDebug >= FREQUENCY_DEBUG;
  if (debugNow) {
        lastDisplayDebug = millis();
  }
  
  // BELIEFS 
  
  bluetooth.readAndReact();

  // pervceive the world
  soundSensor.perceive();
  lightSensor.perceive();
  
  bool isDark = lightSensor.isDark();   // is there any light around? 
  bool isQuiet = soundSensor.isQuiet(); // is there any sound around?
  bool isAmbianceEnabled = ambiance.isEnabled(); // should we play ambiance?
  
  // TODO ask chime what 
  
  // write debug info (from time to time)
   
  if (debugNow) {
    
    clock.debugSerial();
    soundSensor.debugSerial();
    lightSensor.debugSerial();
    ambiance.debugSerial();
    
    DEBUG_PRINT(F("mode: ")); DEBUG_PRINT(mode2str(current_mode));
    if (current_mode != NOTHING) {
      DEBUG_PRINT(F(" in: ")); DEBUG_PRINT((next_planned_action - millis())/1000); DEBUG_PRINT('s');
    }
    DEBUG_PRINTLN();
    //bluetooth.sendDebug();
  }


  // DESIRE 

  // alarms have the power to override past settings (alarm 1 has priority)
  Intention proposedIntention = alarm2.proposeNextMode(current_mode, next_planned_action);
  current_mode = proposedIntention.what; 
  next_planned_action = proposedIntention.when;
  
  proposedIntention = alarm1.proposeNextMode(current_mode, next_planned_action);
  current_mode = proposedIntention.what; 
  next_planned_action = proposedIntention.when;

  // maybe a demo was asked?
  proposedIntention = chime.proposeNextMode(current_mode, next_planned_action);
  current_mode = proposedIntention.what; 
  next_planned_action = proposedIntention.when;

  // maybe the light sensor would like to propose welcoming the sun? 
  proposedIntention = lightSensor.proposeNextMode(current_mode, next_planned_action);
  current_mode = proposedIntention.what; 
  next_planned_action = proposedIntention.when;

  // if nothing happens and we can play ambiance
  proposedIntention = ambiance.proposeNextMode(current_mode, next_planned_action);
  current_mode = proposedIntention.what; 
  next_planned_action = proposedIntention.when;


  if (debugNow) {
    DEBUG_PRINT(F("new mode: ")); DEBUG_PRINT(mode2str(current_mode));
    if (current_mode != NOTHING) {
      DEBUG_PRINT(F(" in: ")); DEBUG_PRINT((next_planned_action - millis())/1000); DEBUG_PRINT('s');
    }
    DEBUG_PRINTLN();
  }

  // INTENTION

  persist.storeIfRequired();

  // maybe it's time to apply what we had planned?
  if (current_mode != NOTHING and next_planned_action <= millis()) {
    DEBUG_PRINTLN(F("time to act!"));
    // time to act
    switch (current_mode) {
      case NOTHING:
      case SILENCE:
          current_mode = NOTHING; 
          break;
      case DEMO_MEDIUM:
      case PREALARM1:
      case PREALARM2:
      case AMBIANCE_PREREVEIL:
          stepper.doPreReveil();
          current_mode = NOTHING;
          break;
      case ALARM1:
      case ALARM2:
      case AMBIANCE_REVEIL:
      case WELCOME_SUN:
      case DEMO_STRONG:
          stepper.doReveil();
          current_mode = NOTHING;
          break;
      case AMBIANCE_TINTEMENT:
      case DEMO_LIGHT:
          stepper.doTintement();
          current_mode = NOTHING;
          break;
    }
   
  }
  
  
}





