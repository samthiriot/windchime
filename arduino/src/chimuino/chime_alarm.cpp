
#include "chime_alarm.h"

#include "debug.h"

#include <Arduino.h>

ChimeAlarm::ChimeAlarm(const char* _name) {
  name = _name;
  name_length = strlen(name);
}

void ChimeAlarm::debugSerial() {
  DEBUG_PRINT("Alarm "); 
  DEBUG_PRINT(name);
  DEBUG_PRINT(": ");
  DEBUG_PRINT(hour); DEBUG_PRINT(":"); DEBUG_PRINT(minutes); DEBUG_PRINT(" ");
  DEBUG_PRINTLN(enabled ? "enabled" : "disabled");
}

BluetoothListenerAnswer ChimeAlarm::processBluetoothGet(char* str, SoftwareSerial* BTSerial) {

  if (strncmp(str, name, name_length) == 0) {
    
    // answer politely
    BTSerial->print(name);
    BTSerial->print(" IS ");
    BTSerial->print(hour,       DEC); BTSerial->print(':');
    BTSerial->print(minutes,    DEC); BTSerial->print(' ');
    BTSerial->println(enabled,  DEC); 
    return SUCCESS;
    
  } 
    
  return NOT_CONCERNED;
  
}

BluetoothListenerAnswer ChimeAlarm::processBluetoothSet(char* str, SoftwareSerial* BTSerial) {
  
  if (strncmp(str, name, name_length) == 0) {

    DEBUG_PRINT("received alarm"); DEBUG_PRINTLN(str);

    // TODO detect errors
    sscanf(
          // skip the name and a space
          str + name_length + 1, 
          // expected format 
          "%u:%u %u", 
          // store directly in our variables
          &hour, &minutes, &enabled);                

    BTSerial->print(name); BTSerial->println(" SET");
    
    // TODO persist somewhere else?
    // TODO activate an alarm in the RTC chip?
    
    debugSerial();

    return SUCCESS;
  }

  return NOT_CONCERNED;

}

BluetoothListenerAnswer ChimeAlarm::processBluetoothDo(char* str, SoftwareSerial* BTSerial) {
  // TODO test alarm?
  return NOT_CONCERNED;
}

bool ChimeAlarm::shouldPrering(DateTime dt) {
  return false;
}

bool ChimeAlarm::shouldRing(DateTime dt) {

  return false;
}


