#include "stepper.h"

#include <Arduino.h>

#include "debug.h"

const char msg_stepper[] PROGMEM  = { "stepper"};

ChimeStepper::ChimeStepper(
      unsigned short _steps, 
      unsigned short _pin1, unsigned short _pin2, unsigned short _pin3, unsigned short _pin4):
        motor(_steps, _pin1,_pin3,_pin2,_pin4) {

  steps = _steps;
  pin1 = _pin1;
  pin2 = _pin2;
  pin3 = _pin3;
  pin4 = _pin4;
        
}
    
void ChimeStepper::setup() {

    DEBUG_PRINT(PGMSTR(message_init)); 
    DEBUG_PRINTLN(PGMSTR(msg_stepper));

    pinMode(pin1, OUTPUT);
    pinMode(pin2, OUTPUT);
    pinMode(pin3, OUTPUT);
    pinMode(pin4, OUTPUT);

    pull_light = steps/16;
    pull_medium = steps/14;
    pull_strong = steps/12;

    pull_light_bck = pull_light/2;
    pull_medium_bck = pull_medium/2;
    pull_strong_bck = pull_strong/2;
    
    TRACE_PRINT(PGMSTR(message_init)); 
    TRACE_PRINTLN(F("going to position free"));
 
    motor.setSpeed(MOTOR_SPEED_SLOW);
    motor.step(pull_medium);
    delay(100); 
    motor.step(-pull_medium_bck);
      
    doFreeWheel(); // relax and don't consume energy

    DEBUG_PRINT(PGMSTR(message_init)); 
    DEBUG_PRINT(PGMSTR(msg_stepper));
    DEBUG_PRINTLN(PGMSTR(msg_ok_dot)); 
}

/**
 * Switch the stepper to free wheel: no noise, no energy consumption.
 */
void ChimeStepper::doFreeWheel() {
  digitalWrite(pin1, LOW);
  digitalWrite(pin2, LOW);
  digitalWrite(pin3, LOW);
  digitalWrite(pin4, LOW); 
}

void ChimeStepper::doReveil() {
  TRACE_PRINTLN(F("reveil..."));
  for (uint8_t i=0; i<random(4,8); i++){
    motor.setSpeed(MOTOR_SPEED_SLOW);
    motor.step(pull_strong);
    delay(200);
    motor.setSpeed(MOTOR_SPEED_SLOW);
    motor.step(-pull_strong_bck);
    delay(200);
  } 
  doFreeWheel();
}
  
void ChimeStepper::doPreReveil() {
  TRACE_PRINTLN(F("prereveil..."));
  for (uint8_t i=0; i<random(3,7); i++){
    motor.setSpeed(MOTOR_SPEED_SLOW);
    motor.step(pull_medium);
    delay(200);
    motor.setSpeed(MOTOR_SPEED_SLOW);
    motor.step(-pull_medium_bck);
    delay(200);
  }
  doFreeWheel(); 
}


void ChimeStepper::doTintement() {
  TRACE_PRINTLN(F("tintement..."));
  for (uint8_t i=0; i<random(1,6); i++){
    motor.setSpeed(MOTOR_SPEED_SLOW);
    motor.step(pull_light);
    delay(200);
    motor.setSpeed(MOTOR_SPEED_SLOW);
    motor.step(-pull_light_bck);
    delay(200);
  }
  doFreeWheel(); 
}
  
