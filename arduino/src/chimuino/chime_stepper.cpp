#include "chime_stepper.h"

#include <Arduino.h>

#include "debug.h"

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
  
    pinMode(pin1, OUTPUT);
    pinMode(pin2, OUTPUT);
    pinMode(pin3, OUTPUT);
    pinMode(pin4, OUTPUT);

    pull_light = steps/11;
    pull_medium = steps/10;
    pull_strong = steps/9;

    DEBUG_PRINTLN("init: going to position free");
 
    motor.setSpeed(MOTOR_SPEED_SLOW);
    motor.step(pull_medium);
    delay(100); 
    motor.step(-pull_medium);
      
    doFreeWheel(); // relax and don't consume energy
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
  DEBUG_PRINTLN("reveil...");
  for (int i=0; i<random(3,7); i++){
    motor.setSpeed(MOTOR_SPEED_QUICK);
    motor.step(pull_strong);
    delay(100);
    motor.setSpeed(MOTOR_SPEED_QUICK);
    motor.step(-pull_strong);
    delay(300);
  } 
  doFreeWheel();
}
  
void ChimeStepper::doPreReveil() {
  DEBUG_PRINTLN("prereveil...");
  for (int i=0; i<random(2,7); i++){
    motor.setSpeed(MOTOR_SPEED_SLOW);
    motor.step(pull_medium);
    delay(100);
    motor.setSpeed(MOTOR_SPEED_QUICK);
    motor.step(-pull_medium);
    delay(300);
  }
  doFreeWheel(); 
}


void ChimeStepper::doTintement() {
  DEBUG_PRINTLN("prereveil...");
  for (int i=0; i<random(1,5); i++){
    motor.setSpeed(MOTOR_SPEED_SLOW);
    motor.step(pull_light);
    delay(100);
    motor.setSpeed(MOTOR_SPEED_QUICK);
    motor.step(-pull_light);
    delay(300);
  }
  doFreeWheel(); 
}
  




