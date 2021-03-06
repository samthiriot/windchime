
#include "alarm.h"

#include "debug.h"
#include "utils.h"

#include <Arduino.h>
#include <Streaming.h>
#include <TimeLib.h>

 
ChimeAlarm::ChimeAlarm(byte _id):
            BluetoothInformationProducer(),
            IntentionProvider() {
  id = _id;
  monday = tuesday = wednesday = thursday = friday = (id == 1);
  saterday = sunday = (id == 2);
  start_hour = id == 1?7:9;
  start_minutes = 30;
}

void ChimeAlarm::setup(Persist* _persist) {

  TRACE_PRINT(message_init); TRACE_PRINT(message_alarm); TRACE_PRINT_DEC(id); TRACE_PRINTLN(F("..."));

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

    TRACE_PRINTLN(PGMSTR(msg_loaded_saved_state));
  } else {
    ERROR_PRINTLN(PGMSTR(msg_not_persisted_default_state));
    storeState();
  }

  DEBUG_PRINT(PGMSTR(message_init)); DEBUG_PRINT(PGMSTR(message_alarm)); DEBUG_PRINT_DEC(id); DEBUG_PRINT(' '); DEBUG_PRINTLN(PGMSTR(msg_ok_dot));
}

void ChimeAlarm::storeState() { 
  if (id == 1) {
    persist->storeAlarm1(enabled, start_hour, start_minutes, durationSoft, durationStrong, sunday, monday, tuesday, wednesday, thursday, friday, saterday);
  } else {
    persist->storeAlarm2(enabled, start_hour, start_minutes, durationSoft, durationStrong, sunday, monday, tuesday, wednesday, thursday, friday, saterday);
  }
}

void ChimeAlarm::debugSerial() {
  #ifdef TRACE
  Serial << message_alarm << id << F(": ") 
         << _DEC(start_hour) << ':' << _DEC(start_minutes) << ' ' 
         << (enabled ? PGMSTR(msg_enabled) : PGMSTR(msg_disabled)) << ' '
         << _DEC(durationSoft) << ' ' << _DEC(durationStrong) << F(" days:")
         << bool2char(sunday) << bool2char(monday) << bool2char(tuesday) << bool2char(wednesday) << bool2char(thursday) << bool2char(friday) << bool2char(saterday)
         << endl;
  #endif 
}

void ChimeAlarm::updateAlarmWithData(ble_alarm content) {

  // store the content as our state
  enabled = content.active;
  start_hour = content.hour;
  start_minutes = content.minutes;
  durationSoft = content.duration_soft;
  durationStrong = content.duration_strong;
  monday = content.monday;
  tuesday = content.tuesday;
  wednesday = content.wednesday;
  thursday = content.thursday;
  friday = content.friday;
  saterday = content.saterday;
  sunday = content.sunday;

  // TODO activate an alarm in the RTC chip?
  debugSerial();
  
  // store this update!
  storeState();
    

}


void ChimeAlarm::publishBluetoothData() {

  // forge the content
  ble_alarm content;
  content.active = enabled;
  content.hour = start_hour;
  content.minutes = start_minutes;
  content.duration_soft = durationSoft;
  content.duration_strong = durationStrong;
  content.monday = monday;
  content.tuesday = tuesday;
  content.wednesday = wednesday;
  content.thursday = thursday;
  content.friday = friday;
  content.saterday = saterday;
  content.sunday = sunday;

  // send it
  if (id == 1) {
    this->bluetooth->publishAlarm1(content);
  } else if (id == 2) {
    this->bluetooth->publishAlarm2(content);
  } else {
    ERROR_PRINTLN(F("ERROR: wrong alarm id"));
  }
}

bool ChimeAlarm::rightWeekdayForRing() {
  uint8_t wd = weekday();
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
  uint16_t preringMinutesStart = start_hour * 60 + start_minutes;
  uint16_t preringMinutesEnd = preringMinutesStart + durationSoft;

  uint16_t currentMinutes = hour() * 60 + minute();
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
  uint16_t ringMinutesStart = start_hour * 60 + start_minutes + durationSoft;
  uint16_t ringMinutesEnd = ringMinutesStart + durationStrong;

  uint16_t currentMinutes = hour() * 60 + minute();
  return (ringMinutesStart <= currentMinutes) and (currentMinutes <= ringMinutesEnd);
  
}

Intention ChimeAlarm::proposeNextMode(Intention currentIntention) {

  enum mode PREALARM = id==1 ? PREALARM1: PREALARM2;
  enum mode ALARM = id==1 ? ALARM1: ALARM2;
  
  if (shouldPrering()) {
    if (currentIntention.what != PREALARM) {
      // we should prering, but we don't; let's propose to act !
      TRACE_PRINT(PGMSTR(message_alarm)); TRACE_PRINT_DEC(id); TRACE_PRINTLN(F(" prering"));
      return Intention { PREALARM,  millis() + random(1*60*1000l,5*60*1000l) };
    }
  } else if (shouldRing()) {
    if (currentIntention.what != ALARM) {
      // we should ring, but we don't; let's propose to ring !
      TRACE_PRINT(PGMSTR(message_alarm)); TRACE_PRINT_DEC(id); TRACE_PRINTLN(F(" ring"));
      return Intention { ALARM,  millis() + random(1*60*1000l,5*60*1000l) };
    }
  } else if (currentIntention.what == PREALARM or currentIntention.what == ALARM) {
    // we were intending to alarm, but it's no time anymore; let's stop and leave some silence
    TRACE_PRINT(F("no more alarm")); TRACE_PRINT(PGMSTR(message_alarm)); TRACE_PRINTLN(id);
    return Intention { SILENCE,  millis() + 60*1000l };
  }

  return currentIntention;
  
}
