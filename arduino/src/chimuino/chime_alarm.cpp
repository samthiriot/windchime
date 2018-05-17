
#include "chime_alarm.h"

#include "debug.h"

#include <Arduino.h>
#include <Streaming.h>
#include <TimeLib.h>

ChimeAlarm::ChimeAlarm(const char* _name) {
  name = _name;
  name_length = strlen(name);
}

void ChimeAlarm::debugSerial() {
  DEBUG_PRINT("Alarm "); 
  DEBUG_PRINT(name);
  DEBUG_PRINT(": ");
  DEBUG_PRINT(start_hour); DEBUG_PRINT(":"); DEBUG_PRINT(start_minutes); DEBUG_PRINT(" ");
  DEBUG_PRINTLN(enabled ? "enabled" : "disabled");
}

BluetoothListenerAnswer ChimeAlarm::processBluetoothGet(char* str, SoftwareSerial* BTSerial) {

  if (strncmp(str, name, name_length) == 0) {
    
    *BTSerial << name << " IS " 
              << start_hour << ":" << start_minutes << " " 
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
          &start_hour, &start_minutes, 
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

bool ChimeAlarm::rightWeekdayForRing() {
  int wd = weekday();
  return ( (wd == 1 and sunday) or 
         (wd == 2 and monday) or 
         (wd == 3 and tuesday) or 
         (wd == 4 and wednesday) or 
         (wd == 5 and thursday) or 
         (wd == 6 and friday) or 
         (wd == 7 and saterday)
         );
}

bool ChimeAlarm::shouldPrering() {

  // TODO manage alarm around midnight

  if (!enabled) {
    return false;
  }
  
  // we only would alarm in case the day is the right one
  if (!rightWeekdayForRing()) {
    return false;
  }

  // define when we should prering based on 
  int preringMinutesStart = start_hour * 60 + start_minutes;
  int preringMinutesEnd = preringMinutesStart + durationSoft;

  int currentMinutes = hour() * 60 + minute();
  return (preringMinutesStart <= currentMinutes) and (currentMinutes <= preringMinutesEnd);
  
}

bool ChimeAlarm::shouldRing() {

  // TODO manage alarm around midnight

  if (!enabled) {
    return false;
  }
  
  // we only would alarm in case the day is the right one
  if (!rightWeekdayForRing()) {
    return false;
  }

   // define when we should prering based on 
  int ringMinutesStart = start_hour * 60 + start_minutes + durationSoft;
  int ringMinutesEnd = ringMinutesStart + durationStrong;

  int currentMinutes = hour() * 60 + minute();
  return (ringMinutesStart <= currentMinutes) and (currentMinutes <= ringMinutesEnd);
  
}


