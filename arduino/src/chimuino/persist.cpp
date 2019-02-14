
#include "persist.h"
#include "debug.h"

#include <EEPROM.h>

Persist::Persist() {
  
};

void Persist::setup() {
  
  EEPROM.get(PERSIST_BASE_ADDRESS, persisted);
  isReadValid = (persisted.magic == PERSIST_MAGIC);
  if (!isReadValid) {
    ERROR_PRINTLN("init persistence: wrong magic, will not read anything !");
    // init values with something which makes sense
    persisted.magic = PERSIST_MAGIC;
    persisted.struct_version = PERSIST_VERSION_CURRENT;
    // the other (nice !) components should tell us soon which default value to write for each parameter.
  } else {
    TRACE_PRINTLN("init persistence: read from EEPROM.");
  }
};


void Persist::storeIfRequired() {
  
  if ((lastUpdate > 0) and (millis()-lastUpdate >= PERSIST_DELAY_MS)) {
    TRACE_PRINTLN(F("Time to persist data into non-volatile memory..."));
    EEPROM.put(PERSIST_BASE_ADDRESS, persisted);
    lastUpdate = 0;
  }
  
}

void Persist::storeAlarm1(
      bool enabled,                                    
      unsigned short start_hour,
      unsigned short start_minutes,
      unsigned short durationSoft,
      unsigned short durationStrong,
      bool sunday,
      bool monday,
      bool tuesday,
      bool wednesday,
      bool thursday,
      bool friday,
      bool saterday) {
        
  persisted.alarm1.enabled = enabled;
  persisted.alarm1.start_hour = start_hour;
  persisted.alarm1.start_minutes = start_minutes;
  persisted.alarm1.durationSoft = durationSoft;
  persisted.alarm1.durationStrong = durationStrong;
  persisted.alarm1.sunday = sunday;
  persisted.alarm1.monday = monday;
  persisted.alarm1.tuesday = tuesday;
  persisted.alarm1.wednesday = wednesday;
  persisted.alarm1.thursday = thursday;
  persisted.alarm1.friday = friday;
  persisted.alarm1.saterday = saterday;
  lastUpdate = millis();
}

void Persist::storeAlarm2(
    bool enabled,                                    
    unsigned short start_hour,
    unsigned short start_minutes,
    unsigned short durationSoft,
    unsigned short durationStrong,
    bool sunday,
    bool monday,
    bool tuesday,
    bool wednesday,
    bool thursday,
    bool friday,
    bool saterday) {
    
  persisted.alarm2.enabled = enabled;
  persisted.alarm2.start_hour = start_hour;
  persisted.alarm2.start_minutes = start_minutes;
  persisted.alarm2.durationSoft = durationSoft;
  persisted.alarm2.durationStrong = durationStrong;
  persisted.alarm2.sunday = sunday;
  persisted.alarm2.monday = monday;
  persisted.alarm2.tuesday = tuesday;
  persisted.alarm2.wednesday = wednesday;
  persisted.alarm2.thursday = thursday;
  persisted.alarm2.friday = friday;
  persisted.alarm2.saterday = saterday;
  lastUpdate = millis();
  
}

void Persist::storeAmbiance(
  bool enabled
  ) {
  persisted.ambianceEnabled = enabled;
  lastUpdate = millis();    
}

void Persist::storeLightThreshold(
  unsigned short lightThreshold
  ) {
  persisted.lightThreshold = lightThreshold;
  lastUpdate = millis();    
}

void Persist::storeSoundThreshold(
  unsigned short soundThreshold
  ) {
  persisted.soundThreshold = soundThreshold;
  lastUpdate = millis();
}
 
