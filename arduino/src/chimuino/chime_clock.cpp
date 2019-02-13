
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

void ChimeClock::publishBluetoothData() {

    // publish our temperature
    float c = RTC.temperature() / 4.;
    bluetooth->publishTemperature1(c);

    // publish the uptime
    // TODO this uptime overflows quickly. use RTC instead?
    uint32_t uptime = (millis() - startTimestamp)/(60l*1000l);
    bluetooth->publishUptime(uptime);

    
}

BluetoothListenerAnswer ChimeClock::receivedCurrentDateTime(ble_datetime content) {

  // forge data to send it into the chip
  time_t t;         // the time to forge
  tmElements_t tm;
  tm.Year =   CalendarYrToTm(content.year);
  tm.Month =  content.month;
  tm.Day =    content.day;
  tm.Hour =   content.hour;
  tm.Minute = content.minutes;
  tm.Second = content.seconds;
  t = makeTime(tm);

  // storage inside the chip
  RTC.set(t);
  setTime(t); // ... and also as arduino time ^^

  // inform
  debugSerial();

  return PROCESSED;
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

