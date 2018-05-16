
#include "chime_clock.h"

#include "debug.h"

ChimeClock::ChimeClock() {
  
}

void ChimeClock::setup() {
  
  // init RTC !
  DEBUG_PRINTLN("init: RTC...");
  
  if (! rtc.begin()) {
    DEBUG_PRINTLN("ERROR: Couldn't find RTC");
    while (1); // TODO
  }
  
  DEBUG_PRINTLN("Found RTC :-)");

  if (rtc.lostPower()) {
     DEBUG_PRINTLN("WARN: RTC is NOT running! Initializing with compilation time.");
     rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));                    //  sets the RTC to the date & time this sketch was compiled
  }
  
  DEBUG_PRINTLN("init: RTC ok");

}

DateTime ChimeClock::now() {
  return rtc.now();
}

void ChimeClock::debugSerial() {
  DEBUG_PRINT("Time is: ");
  DateTime now = rtc.now();
  DEBUG_PRINT_DEC(now.year());    DEBUG_PRINT('-');
  DEBUG_PRINT_DEC(now.month());   DEBUG_PRINT('-');
  DEBUG_PRINT_DEC(now.day());     DEBUG_PRINT(' ');
  DEBUG_PRINT_DEC(now.hour());    DEBUG_PRINT(':');
  DEBUG_PRINT_DEC(now.minute());  DEBUG_PRINT(':');
  DEBUG_PRINT_DEC(now.second());  DEBUG_PRINTLN();
}


BluetoothListenerAnswer ChimeClock::processBluetoothGet(char* str, SoftwareSerial* BTSerial) {
  
  if (strncmp(str, "DATETIME", 8) == 0) {
    BTSerial->print("DATETIME IS ");
    DateTime now = rtc.now();
    BTSerial->print(now.year(),     DEC); BTSerial->print('-');
    BTSerial->print(now.month(),    DEC); BTSerial->print('-');
    BTSerial->print(now.day(),      DEC); BTSerial->print(' ');
    BTSerial->print(now.hour(),     DEC); BTSerial->print(':');
    BTSerial->print(now.minute(),   DEC); BTSerial->print(':');
    BTSerial->println(now.second(), DEC);
    return SUCCESS;
  }

  return NOT_CONCERNED;
}

BluetoothListenerAnswer ChimeClock::processBluetoothSet(char* str, SoftwareSerial* BTSerial) {
  
  if (strncmp(str, "DATETIME ", 9) == 0) {

    DEBUG_PRINT("received time: "); DEBUG_PRINTLN(str);

    int year, month, day, hour, minutes, seconds;
    sscanf(str + 9,                                                  // decode received datetime
           "%u-%u-%u %u:%u:%u", 
           &year, &month, &day, &hour, &minutes, &seconds);    
           
    // TODO reject invalid time 
    rtc.adjust(DateTime(year, month, day, hour, minutes, seconds));  // store the novel datetime into the RTC clock
    
    BTSerial->write("DATETIME SET\n");                               // acknowledge
      
    debugSerial();

    return SUCCESS;
  }

  return NOT_CONCERNED;
}

BluetoothListenerAnswer ChimeClock::processBluetoothDo(char* str,  SoftwareSerial* BTSerial) {
  
}






