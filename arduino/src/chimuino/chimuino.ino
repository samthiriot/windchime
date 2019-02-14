// about bluetooth
// docs
// - https://medium.com/@yostane/using-the-at-09-ble-module-with-the-arduino-3bc7d5cb0ac2

/*
 * TODO
 * 
 * connectable
 * when a button is pressed, then:
 * - delete former bonds AT+GAPDELBONDS
 * - make BLE connectable AT+GAPCONNECTABLE   see https://learn.adafruit.com/introducing-the-adafruit-bluefruit-le-uart-friend/ble-gap
 * blink a led or something so the user knows it is ok???
 * 
 */

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

// frequency for displaying debug messages on the state
#define FREQUENCY_DEBUG 5000
long lastDisplayDebug = millis();


// hardware settings

// pins for bluetooth

#define BLUEFRUIT_UART_CTS_PIN    9
#define BLUEFRUIT_UART_MODE_PIN   8   // optional, set to -1 if unused
#define BLUEFRUIT_SWUART_TXD_PIN  11
#define BLUEFRUIT_SWUART_RXD_PIN  10
#define BLUEFRUIT_UART_RTS_PIN    12  // optional, set to -1 if unused

// pins for the stepper engine
#define MOTOR_TOTAL_STEPS 2048                                    // in 4 steps mode
#define STEPPER_PIN_1 4
#define STEPPER_PIN_2 5
#define STEPPER_PIN_3 6
#define STEPPER_PIN_4 7

// pins for the RTC 3132 bug which plugs in I2C.
#define SDA A4
#define SCL A5

#define SOUND_PIN A1
#define PHOTOCELL_PIN A0                                                  // port for the photovoltaic cell

#define RANDOM_PIN A2                                                     // refers to a pin unconnected supposed to catch white noise 

Persist persist;      // persistence object

ChimeStepper stepper(MOTOR_TOTAL_STEPS, STEPPER_PIN_1, STEPPER_PIN_2, STEPPER_PIN_3, STEPPER_PIN_4);

ChimeClock clock;

ChimeAlarm alarm1(1);
ChimeAlarm alarm2(2);

ChimeBluetooth bluetooth(BLUEFRUIT_SWUART_TXD_PIN, BLUEFRUIT_SWUART_RXD_PIN, BLUEFRUIT_UART_MODE_PIN, BLUEFRUIT_UART_CTS_PIN, BLUEFRUIT_UART_CTS_PIN);

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
    // TODO use BLE dongle instead?
    int seed = analogRead(RANDOM_PIN);
    TRACE_PRINT(F("init: random seed is ")); TRACE_PRINTLN(seed); 
    randomSeed(seed);
}


unsigned long next_planned_action = millis();

mode current_mode = NOTHING;


/**
 * Entry point, ran before anything else by the Arduino.
 */
void setup() {
  
  DEBUG_SETUP();
  
  delay(500);
  
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
  clock.setBluetooth(&bluetooth);
  alarm1.setBluetooth(&bluetooth);
  alarm2.setBluetooth(&bluetooth);
  lightSensor.setBluetooth(&bluetooth);
  soundSensor.setBluetooth(&bluetooth);
  chime.setBluetooth(&bluetooth);
  ambiance.setBluetooth(&bluetooth);

  // inform bluetooth of the 
  bluetooth.setUsers(&clock, &alarm1, &alarm2, &lightSensor, &soundSensor, &chime, &ambiance);
  
  // plugin 

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





