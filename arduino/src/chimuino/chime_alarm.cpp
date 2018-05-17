
#include "chime_alarm.h"

#include "debug.h"

#include <Arduino.h>
#include <Streaming.h>

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
    
    *BTSerial << name << " IS " 
              << hour << ":" << minutes << " " 
              << (enabled ? '1':'0') << ' ' 
              << (sunday ? '1':'0') << (monday ? '1':'0') << (tuesday ? '1':'0') << (wednesday ? '1':'0') << (thursday ? '1':'0') << (friday ? '1':'0') << (saterday ? '1':'0')
              << endl;
              
    return SUCCESS;
  } 
    
  return NOT_CONCERNED;
}

BluetoothListenerAnswer ChimeAlarm::processBluetoothSet(char* str, SoftwareSerial* BTSerial) {
  
  if (strncmp(str, name, name_length) == 0) {

    DEBUG_PRINT("received alarm"); DEBUG_PRINTLN(str);

    char cEnabled;
    char cSunday, cMonday, cTuesday, cWednesday, cThursday, cFriday, cSaterday;
    // TODO detect errors
    sscanf(
          // skip the name and a space
          str + name_length + 1, 
          // expected format 
          "%u:%u %c %c%c%c%c%c%c%c", 
          // store directly in our variables
          &hour, &minutes, 
          &cEnabled,
          &cSunday, &cMonday, &cTuesday, &cWednesday, &cThursday, &cFriday, &cSaterday
          );                

    // decode chars
    enabled = (cEnabled == '1');
    sunday = (cSunday == '1');
    monday = (cMonday == '1');
    tuesday = (cTuesday == '1');
    wednesday = (cWednesday == '1');
    thursday = (cThursday == '1');
    friday = (cFriday == '1');
    saterday = (cSaterday == '1');

    *BTSerial << name << " SET" << endl;
    
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


