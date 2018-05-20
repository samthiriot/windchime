// about bluetooth
// docs
// - https://medium.com/@yostane/using-the-at-09-ble-module-with-the-arduino-3bc7d5cb0ac2

#include "debug.h"

#include "chime.h"
#include "chime_bluetooth.h"
#include "chime_stepper.h"
#include "chime_clock.h"
#include "chime_alarm.h"
#include "chime_soundsensor.h"
#include "chime_lightsensor.h"


// sketch settings

// ... current version of the firmware.
//     should be upgraded at update time
#define FIRMWARE_VERSION "alpha_2018_05_16"

#define DEBUG_SERIAL true                                         // config debug

// frequency for displaying debug messages on the state
#define FREQUENCY_DEBUG 2000
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

ChimeStepper stepper(MOTOR_TOTAL_STEPS, STEPPER_PIN_1, STEPPER_PIN_2, STEPPER_PIN_3, STEPPER_PIN_4);

ChimeClock clock;

ChimeAlarm alarm1("ALARM1");
ChimeAlarm alarm2("ALARM2");

ChimeBluetooth bluetooth(BLUETOOTH_RX, BLUETOOTH_TX);

ChimeSoundSensor soundSensor(SOUND_PIN);
ChimeLightSensor lightSensor(PHOTOCELL_PIN);
   
// +---------------------------------------------+
// |      LIGHT SENSOR                           |
// |            simple but smart                 |
// +---------------------------------------------+
    
   
bool wasDark = false;                                                     // true if was dark before


// +---------------------------------------------+
// |      RANDOM                                 |
// |            alea yacta est                   |
// +---------------------------------------------+
    
    
    void setupRandom() {
        pinMode(RANDOM_PIN, INPUT);                                           // this pin is used to init the random network generator
        int seed = analogRead(RANDOM_PIN);
      #ifdef DEBUG_SERIAL
        Serial.print("init: random seed is "); Serial.print(seed); Serial.println();
      #endif
        randomSeed(seed);
    }

bool ambiance = true;             // play sound from time to time 

unsigned long next_planned_action = millis();

mode current_mode = NOTHING;


/**
 * Entry point, ran before anything else by the Arduino.
 */
void setup() {
  
  DEBUG_SETUP();

  setupRandom();

  soundSensor.setup();
  lightSensor.setup();
  stepper.setup();
  clock.setup();
  alarm1.setup();
  alarm2.setup();
  bluetooth.setup();

  // ... add the chain of listeners
  bluetooth.addCommandInterpreter(&alarm1);
  bluetooth.addCommandInterpreter(&alarm2);
  bluetooth.addCommandInterpreter(&clock);
  bluetooth.addCommandInterpreter(&lightSensor);
  bluetooth.addCommandInterpreter(&soundSensor);

  DEBUG_PRINTLN("init: end.");

}

void loop() {

  // BELIEFS 
  
  bluetooth.readAndReact();

  // pervceive the world
  soundSensor.perceive();
  lightSensor.perceive();
  
  bool isDark = lightSensor.isDark();   // is there any light around? 
  bool isQuiet = soundSensor.isQuiet(); // is there any sound around?
  //DateTime now = clock.now();           // what time is it ? 
  
  // write debug info (from time to time)
  if (millis() - lastDisplayDebug >= FREQUENCY_DEBUG) {
    clock.debugSerial();
    soundSensor.debugSerial();
    lightSensor.debugSerial();
    lastDisplayDebug = millis();
    DEBUG_PRINT(F("mode: ")); DEBUG_PRINTLN(mode2str(current_mode));
    if (current_mode != NOTHING) {
      DEBUG_PRINT(F("next action in: ")); DEBUG_PRINTLN((next_planned_action - millis())/1000);
    }
  }

  // DESIRE 

  // if an alarm should be rang, then override wathever current setting
  if (alarm1.shouldPrering())      { if (current_mode != PREALARM1)  { current_mode = PREALARM1;   next_planned_action = millis() + random(1*60*1000l,5*60*1000l); DEBUG_PRINTLN("alarm1 prering"); } }
  else if (alarm2.shouldPrering()) { if (current_mode != PREALARM2)  { current_mode = PREALARM2;   next_planned_action = millis() + random(1*60*1000l,5*60*1000l); DEBUG_PRINTLN("alarm2 prering"); } }
  else if (alarm1.shouldRing())    { if (current_mode != ALARM1)     { current_mode = ALARM1;      next_planned_action = millis() + random(1*60*1000l,5*60*1000l); DEBUG_PRINTLN("alarm1 ring"); } }
  else if (alarm2.shouldRing())    { if (current_mode != ALARM2)     { current_mode = ALARM2;      next_planned_action = millis() + random(1*60*1000l,5*60*1000l); DEBUG_PRINTLN("alarm2 ring"); } }
  else if (current_mode == PREALARM1 or current_mode == PREALARM2 or current_mode == ALARM1 or current_mode == ALARM2) {
    // no alarm asked anymore, so it's not our current mode anymore
    current_mode = NOTHING;
    DEBUG_PRINTLN(F("no more alarm."));
  }

  // if light just came, then celebrate it
  if (current_mode == NOTHING and !isDark and wasDark) {
      current_mode = WELCOME_SUN;
      DEBUG_PRINTLN(F("welcoming the sun ;-)"));
      next_planned_action = millis();
  }

  // if nothing happens and we can play ambiance
  if (current_mode == NOTHING and ambiance and !isDark and isQuiet) {
    int r = random(0,100);
    if (r <= 60) { 
      current_mode = SILENCE;
      next_planned_action = millis() + random(60*1000l,15*60*1000l);
      DEBUG_PRINT(F("a bit of silence for ")); DEBUG_PRINTLN((next_planned_action - millis())/1000);
    } else if (r <= 65) {
      current_mode = AMBIANCE_REVEIL;
      next_planned_action = millis() + random(4*60*1000l,15*60*1000l);
      DEBUG_PRINT(F("mood strong in ")); DEBUG_PRINTLN((next_planned_action - millis())/1000);
    } else if (r <= 75) {
      current_mode = AMBIANCE_PREREVEIL;
      next_planned_action = millis() + random(4*60*1000l,10*60*1000l);  
      DEBUG_PRINT(F("mood medium in ")); DEBUG_PRINTLN((next_planned_action - millis())/1000);
    } else {
      current_mode = AMBIANCE_TINTEMENT;
      next_planned_action = millis() + random(1*10*1000l,7*60*1000l);
      DEBUG_PRINT(F("mood slight in ")); DEBUG_PRINTLN((next_planned_action - millis())/1000);
    }
  }
  
  
  // MEMORIZE THE WORLD
  wasDark = isDark;

  // INTENTION

  // maybe it's time to apply what we had planned?
  if (next_planned_action >= millis()) {
    // time to act
    switch (current_mode) {
      case NOTHING:
      case SILENCE:
          break;
      case PREALARM1:
      case PREALARM2:
      case AMBIANCE_PREREVEIL:
        stepper.doPreReveil();
        break;
      case ALARM1:
      case ALARM2:
      case AMBIANCE_REVEIL:
      case WELCOME_SUN:
        stepper.doReveil();
        break;
      case AMBIANCE_TINTEMENT:
        stepper.doTintement();
        break;
    }
    
  }
  
  
}





