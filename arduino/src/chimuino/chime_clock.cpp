
#include "chime_clock.h"

#include "debug.h"
#include <Streaming.h>

ChimeClock::ChimeClock() {
  // nothing to do (crazy, huh?)
  // as everything is done by the DS3232RTC library
}

// function to return the compile date and time as a time_t value
time_t compileTime()
{
    const time_t FUDGE(10);    //fudge factor to allow for upload time, etc. (seconds, YMMV)
    char *compDate = __DATE__, *compTime = __TIME__, *months = "JanFebMarAprMayJunJulAugSepOctNovDec";
    char compMon[3], *m;

    strncpy(compMon, compDate, 3);
    compMon[3] = '\0';
    m = strstr(months, compMon);

    tmElements_t tm;
    tm.Month = ((m - months) / 3 + 1);
    tm.Day = atoi(compDate + 4);
    tm.Year = atoi(compDate + 7) - 1970;
    tm.Hour = atoi(compTime);
    tm.Minute = atoi(compTime + 3);
    tm.Second = atoi(compTime + 6);

    time_t t = makeTime(tm);
    return t + FUDGE;        //add fudge factor to allow for compile time
}


void ChimeClock::setup() {
  
  DEBUG_PRINTLN(F("init: RTC..."));
  
  // tells the TimeLib library that time is provided by our RTC chip
  setSyncProvider(RTC.get);
  setSyncInterval(1);         // only one second for resync

  if (timeStatus() == timeNotSet or RTC.oscStopped(false)) {
     DEBUG_PRINTLN(F("WARN: RTC is NOT running! Initializing with compilation time."));
     time_t compilation_time = compileTime();
     RTC.set(compilation_time);
     setTime(compilation_time);
  }

  // disable interrupts
  // TODO maybe to reactivate to space arduino consumption
  RTC.alarmInterrupt(ALARM_1, false);
  RTC.alarmInterrupt(ALARM_2, false);

  // save power by disabling the square wave
  RTC.squareWave(SQWAVE_NONE);

  startTimestamp = millis();
  
  DEBUG_PRINTLN(F("init: RTC ok"));

}

time_t ChimeClock::now() {
  return RTC.get();
}

void ChimeClock::debugSerial() {

  #ifdef DEBUG
  time_t now = RTC.get();
    Serial << F("Time is: ")
           << _DEC(year(now))  << '-' << _DEC(month(now)) << '-' << _DEC(day(now)) << ' '
           << _DEC(hour(now)) << ':' << _DEC(minute(now)) << ':' << _DEC(second(now)) 
           << endl;
  #endif
}

float ChimeClock::getTemperature() {
  float c = RTC.temperature() / 4.;
  return c;
}

BluetoothListenerAnswer ChimeClock::processBluetoothGet(char* str, SoftwareSerial* BTSerial) {
  
  if (strncmp_P(str, PSTR("DATETIME"), 8) == 0) {

    time_t now = RTC.get();
    *BTSerial << F("DATETIME IS ") 
              << _DEC(year(now))  << '-' << _DEC(month(now)) << '-' << _DEC(year(now)) << ' '
              << _DEC(hour(now)) << ':' << _DEC(minute(now)) << ':' << _DEC(second(now)) 
              << endl;
 
    return SUCCESS;
  }

  if (strncmp_P(str, PSTR("UPTIME"), 6) == 0) {

    *BTSerial << F("UPTIME IS ") 
              << (millis() - startTimestamp)/(60l*1000l)
              << endl;

    DEBUG_PRINTLN(F("SENT VERSION"));
     
    return SUCCESS;
  }


  if (strncmp_P(str, PSTR("TEMPERATURE"), 11) == 0) {
    float temp = getTemperature();
     *BTSerial << F("TEMPERATURE IS ") << temp << endl;
     return SUCCESS;
  }

  return NOT_CONCERNED;
}

BluetoothListenerAnswer ChimeClock::processBluetoothSet(char* str, SoftwareSerial* BTSerial) {

  if (strncmp_P(str, PSTR("DATETIME "), 9) == 0) {
    DEBUG_PRINT(F("received datetime: ")); DEBUG_PRINTLN(str);
    time_t t;     // the time to forge
    tmElements_t tm;

    int year, month, day, hour, minute, second;
    sscanf(str + 9,                                                  // decode received datetime
           "%u-%u-%u %u:%u:%u", 
           &year, &month, &day, &hour, &minute, &second);    
    tm.Year = CalendarYrToTm(year);
    tm.Month = month;
    tm.Day = day;
    tm.Hour = hour;
    tm.Minute = minute;
    tm.Second = second;
    t = makeTime(tm);
    RTC.set(t);
    setTime(t);
    *BTSerial << F("DATETIME SET") << endl;
    debugSerial();

    return SUCCESS;
  }

  return NOT_CONCERNED;
}

BluetoothListenerAnswer ChimeClock::processBluetoothDo(char* str,  SoftwareSerial* BTSerial) {
  // TODO ???
  return NOT_CONCERNED;
}

/* TODO 
void ChimeClock::setAlarm1(unsigned short hour, unsigned short minutes) {
  RTC.setAlarm(
    ALM1_MATCH_HOURS,   // match hours, minutes, seconds
    minutes, hour, 
    (byte)0             // nothing for the day
    );
}
*/

