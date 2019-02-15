#ifndef CHIME_PERSIST_H
#define CHIME_PERSIST_H

#include <Arduino.h>

// write data after 5 min without change
#define PERSIST_DELAY_MS 1000l*60*5

#define PERSIST_BASE_ADDRESS 0
#define PERSIST_MAGIC 501983
#define PERSIST_VERSION_CURRENT 1;

const char msg_not_persisted_default_state[] PROGMEM  = { "no saved state, defining the default state"};
const char msg_loaded_saved_state[] PROGMEM  = { "loaded data from saved state"};


struct ChimePersistedAlarmData {
  bool enabled;
  unsigned short start_hour;
  unsigned short start_minutes;
  unsigned short durationSoft;
  unsigned short durationStrong;
  bool sunday;
  bool monday;
  bool tuesday;
  bool wednesday;
  bool thursday;
  bool friday;
  bool saterday; 
};

struct ChimePersistedData {
  uint32_t magic; 
  byte struct_version;
  ChimePersistedAlarmData alarm1;
  ChimePersistedAlarmData alarm2;
  unsigned short lightThreshold;
  unsigned short soundThreshold;
  bool ambianceEnabled;
};

class Persist {
 
  private:
    unsigned long lastUpdate = 0;
    ChimePersistedData persisted;
    bool isReadValid = false;
    
  public:
    // constructor
    Persist();
    void setup();

    // will store an update if relevant
    void storeIfRequired();
    
    void debugSerial();

    // returns true if there is something to read
    bool hasDataStored() { return isReadValid; }

    ChimePersistedAlarmData getAlarm1() { return persisted.alarm1; };
    ChimePersistedAlarmData getAlarm2() { return persisted.alarm2; };

    unsigned short getLightThreshold()    { return persisted.lightThreshold;    };
    unsigned short getSoundThreshold()    { return persisted.soundThreshold;    };
    bool getAmbianceEnabled()             { return persisted.ambianceEnabled;   };

    
    void storeAlarm1(
      bool enabled,                                    
      unsigned short start_hour,                               // the hour 
      unsigned short start_minutes,
      unsigned short durationSoft,
      unsigned short durationStrong,
      bool sunday,
      bool monday,
      bool tuesday,
      bool wednesday,
      bool thursday,
      bool friday,
      bool saterday);

    void storeAlarm2(
      bool enabled,                                    
      unsigned short start_hour,                               // the hour 
      unsigned short start_minutes,
      unsigned short durationSoft,
      unsigned short durationStrong,
      bool sunday,
      bool monday,
      bool tuesday,
      bool wednesday,
      bool thursday,
      bool friday,
      bool saterday);

    void storeAmbiance(
      bool enabled
      );

    void storeLightThreshold(
      unsigned short lightThreshold
      );

    void storeSoundThreshold(
      unsigned short soundThreshold
      );
     
     
};

#endif // CHIME_PERSIST_H





