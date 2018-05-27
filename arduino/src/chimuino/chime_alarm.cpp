
#include "chime_alarm.h"

#include "debug.h"
#include "utils.h"

#include <Arduino.h>
#include <Streaming.h>
#include <TimeLib.h>

ChimeAlarm::ChimeAlarm(byte _id):
            BluetoothCommandListener(),
            IntentionProvider() {
  id = _id;
  monday = tuesday = wednesday = thursday = friday = (id == 1);
  saterday = sunday = (id == 2);
  start_hour = id == 1?7:9;
  start_minutes = 30;
}

void ChimeAlarm::setup(Persist* _persist) {

  DEBUG_PRINT(F("init: ALARM")); DEBUG_PRINT_DEC(id); DEBUG_PRINTLN(F("..."));

  persist = _persist; 
  if (persist->hasDataStored()) {
    // there is data persisted ! Let's load it :-)
    ChimePersistedAlarmData read = (id==1)? persist->getAlarm1() : persist->getAlarm2();      
    enabled = read.enabled;
    start_hour = read.start_hour;
    start_minutes = read.start_minutes;
    durationSoft = read.durationSoft;
    durationStrong = read.durationStrong;
    sunday = read.sunday;
    monday = read.monday;
    tuesday = read.tuesday;
    wednesday = read.wednesday;
    thursday = read.thursday;
    friday = read.friday;
    saterday = read.saterday;
    DEBUG_PRINTLN(F("loaded data from saved state"));
  } else {
    DEBUG_PRINTLN(F("no saved state, defining the default state..."));
    storeState();
  }

  DEBUG_PRINT(F("init: ALARM")); DEBUG_PRINT_DEC(id); DEBUG_PRINTLN(F(" ok"));
}

void ChimeAlarm::storeState() { 
  if (id == 1) {
    persist->storeAlarm1(enabled, start_hour, start_minutes, durationSoft, durationStrong, sunday, monday, tuesday, wednesday, thursday, friday, saterday);
  } else {
    persist->storeAlarm2(enabled, start_hour, start_minutes, durationSoft, durationStrong, sunday, monday, tuesday, wednesday, thursday, friday, saterday);
  }
}

void ChimeAlarm::debugSerial() {
  #ifdef DEBUG
  Serial << F("Alarm ALARM") << id << F(": ") 
         << _DEC(start_hour) << ':' << _DEC(start_minutes) << ' ' 
         << (enabled ? F("enabled") : F("disabled")) << ' '
         << _DEC(durationSoft) << ' ' << _DEC(durationStrong) << F(" days:")
         << bool2char(sunday) << bool2char(monday) << bool2char(tuesday) << bool2char(wednesday) << bool2char(thursday) << bool2char(friday) << bool2char(saterday)
         << endl;
  #endif 
}


BluetoothListenerAnswer ChimeAlarm::processBluetoothGet(char* str, SoftwareSerial* BTSerial) {

  if ( (strncmp_P(str, PSTR("ALARM"), 5) == 0) and (str[5] == (id==1?'1':'2') ) ) {
    
    *BTSerial << F("ALARM") << id << F(" IS ")
              << start_hour << ':' << start_minutes << ' ' 
              << durationSoft << ' ' << durationStrong << ' '
              << bool2char(enabled) << ' ' 
              << bool2char(sunday) << bool2char(monday) << bool2char(tuesday) << bool2char(wednesday) << bool2char(thursday) << bool2char(friday) << bool2char(saterday)
              << endl;

    DEBUG_PRINT(F("SEND ")); debugSerial();
    
    return SUCCESS;
  } 
    
  return NOT_CONCERNED;
}


BluetoothListenerAnswer ChimeAlarm::processBluetoothSet(char* str, SoftwareSerial* BTSerial) {
  
  if ( (strncmp_P(str, PSTR("ALARM"), 5) == 0) and (str[5] == (id==1?'1':'2') ) ) {

    DEBUG_PRINT(F("received alarm")); DEBUG_PRINTLN(str);

    char cEnabled;
    char cSunday, cMonday, cTuesday, cWednesday, cThursday, cFriday, cSaterday;
    // TODO detect errors
    sscanf(
          // skip the name and a space
          str + 5 + 1, 
          // expected format 
          "%u:%u %u %u %c %c%c%c%c%c%c%c", 
          // store directly in our variables
          &start_hour, &start_minutes, 
          &durationSoft, &durationStrong,
          &cEnabled,
          &cSunday, &cMonday, &cTuesday, &cWednesday, &cThursday, &cFriday, &cSaterday
          );                

    // decode chars
    enabled =   char2bool(cEnabled);
    
    sunday =    char2bool(cSunday);
    monday =    char2bool(cMonday);
    tuesday =   char2bool(cTuesday);
    wednesday = char2bool(cWednesday);
    thursday =  char2bool(cThursday);
    friday =    char2bool(cFriday);
    saterday =  char2bool(cSaterday);

    if (start_hour > 23) { start_hour = 23; }
    if (start_minutes > 59) { start_minutes = 59; }

    *BTSerial << F("ALARM") << id << F(" SET") << endl;
    
    storeState();
    
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

Intention ChimeAlarm::proposeNextMode(enum mode current_mode, unsigned long next_planned_action) {

  enum mode PREALARM = id==1 ? PREALARM1: PREALARM2;
  enum mode ALARM = id==1 ? ALARM1: ALARM2;
  
  if (shouldPrering()) {
    if (current_mode != PREALARM) {
      // we should prering, but we don't; let's propose to act !
      DEBUG_PRINT(F("alarm")); DEBUG_PRINT_DEC(id); DEBUG_PRINTLN(F(" prering"));
      return Intention { PREALARM,  millis() + random(1*60*1000l,5*60*1000l) };
    }
  } else if (shouldRing()) {
    if (current_mode != ALARM) {
      // we should ring, but we don't; let's propose to ring !
      DEBUG_PRINT(F("alarm")); DEBUG_PRINT_DEC(id); DEBUG_PRINTLN(F(" ring"));
      return Intention { ALARM,  millis() + random(1*60*1000l,5*60*1000l) };
    }
  } else if (current_mode == PREALARM or current_mode == ALARM) {
    // we were intending to alarm, but it's no time anymore; let's stop and leave some silence
    DEBUG_PRINT(F("no more alarm ALARM"));DEBUG_PRINTLN(id);
    return Intention { SILENCE,  millis() + 60*1000l };
  }

  return Intention { current_mode, next_planned_action };
  
}


