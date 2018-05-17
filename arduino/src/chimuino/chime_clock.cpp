
#include "chime_clock.h"

#include "debug.h"
#include <Streaming.h>

ChimeClock::ChimeClock() {
  
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
  
  // init RTC !
  DEBUG_PRINTLN("init: RTC...");


  // tells the TimeLib library that time is provided by our RTC chip
  setSyncProvider(RTC.get);
  setSyncInterval(1);         // only one second for resync

  if (timeStatus() == timeNotSet) {
     DEBUG_PRINTLN(F("WARN: RTC is NOT running! Initializing with compilation time."));
     time_t compilation_time = compileTime();
     RTC.set(compilation_time);
     setTime(compilation_time);
  }

  // disable interrupts
  RTC.alarmInterrupt(ALARM_1, false);
  RTC.alarmInterrupt(ALARM_2, false);

  DEBUG_PRINTLN("init: RTC ok");

}

time_t ChimeClock::now() {
  return RTC.get();
}

void ChimeClock::debugSerial() {

  #ifdef DEBUG
  time_t now = RTC.get();
    Serial << "Time is: " 
           << _DEC(year(now))  << '-' << _DEC(month(now)) << '-' << _DEC(year(now)) << ' '
           << _DEC(hour(now)) << ':' << _DEC(minute(now)) << ':' << _DEC(second(now)) 
           << endl;
  #endif
}

float ChimeClock::getTemperature() {
  float c = RTC.temperature() / 4.;
  return c;
}

BluetoothListenerAnswer ChimeClock::processBluetoothGet(char* str, SoftwareSerial* BTSerial) {
  
  if (strncmp(str, "DATETIME", 8) == 0) {

    time_t now = RTC.get();
    *BTSerial << "DATETIME IS " 
              << _DEC(year(now))  << '-' << _DEC(month(now)) << '-' << _DEC(year(now)) << ' '
              << _DEC(hour(now)) << ':' << _DEC(minute(now)) << ':' << _DEC(second(now)) 
              << endl;
 
    return SUCCESS;
  }

  if (strncmp(str, "TEMPERATURE", 11) == 0) {
    float temp = getTemperature();
     *BTSerial << F("TEMPERATURE IS ") << temp << endl;
     return SUCCESS;
  }

  return NOT_CONCERNED;
}

BluetoothListenerAnswer ChimeClock::processBluetoothSet(char* str, SoftwareSerial* BTSerial) {
  
  if (strncmp(str, "DATETIME ", 9) == 0) {

    DEBUG_PRINT(F("received time: ")); DEBUG_PRINTLN(str);

    time_t t;     // the time to forge
    tmElements_t tm;

    int year;
    sscanf(str + 9,                                                  // decode received datetime
           "%u-%u-%u %u:%u:%u", 
           &year, &tm.Month, &tm.Day, &tm.Hour, &tm.Minute, &tm.Second);    

    tm.Year = CalendarYrToTm(year);
    t = makeTime(tm);

    // TODO reject invalid time 
    
    RTC.set(t);  // store the novel datetime into the RTC clock
    setTime(t);
    BTSerial->println(F("DATETIME SET"));                               // acknowledge
      
    debugSerial();

    return SUCCESS;
  }

  return NOT_CONCERNED;
}

BluetoothListenerAnswer ChimeClock::processBluetoothDo(char* str,  SoftwareSerial* BTSerial) {
  
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

