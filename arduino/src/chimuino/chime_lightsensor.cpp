#include "chime_lightsensor.h"

#include <Arduino.h>
#include <Streaming.h>

#include "debug.h"

ChimeLightSensor::ChimeLightSensor(const byte _pin):
              BluetoothUser(),
              IntentionProvider(),
              sensor( CHIME_LIGHTSENSOR_MEASURES_ETA, _pin, 
                      CHIME_LIGHTSENSOR_MEASURES_FREQUENCY,// read every 100ms
                      CHIME_LIGHTSENSOR_ENVELOPE_ETA_SLOW, CHIME_LIGHTSENSOR_ENVELOPE_ETA_QUICK
                      )  
              {

  // save params
  pin = _pin;

  // TODO reload settings from storage!!!

}

void ChimeLightSensor::perceive() {
  
  if (sensor.sense()) {
    // the sensor was updated

    // ... update the threshold!
    darkThreshold = sensor.envelopeMin() + int(float(factorThreshold) * float(sensor.envelopeMax() - sensor.envelopeMin()) / 100.);
  
  }
}

void ChimeLightSensor::debugSerial() {

  #ifdef DEBUG
  Serial << F("LIGHTLEVEL IS ") 
         << _DEC(getLightLevel()) << ' ' 
         << (isDark() ? F("DARK"): F("LIT")) << ','
         << _DEC(factorThreshold) << '>'
         << _DEC(darkThreshold) << F(" in ")
         << '[' << _DEC(sensor.envelopeMin()) << ':' << _DEC(sensor.envelopeMax()) << ']'
         << endl;
  #endif
}

void ChimeLightSensor::setup(Persist* _persist) {

  DEBUG_PRINTLN(F("init: light sensor..."));

  sensor.setup(0, -1); // we assume no light means 0, but have no idea of the max
  sensor.sense();

  persist = _persist; 
  if (persist->hasDataStored()) {
    // there is data persisted ! Let's load it :-)
    factorThreshold = persist->getLightThreshold();
    DEBUG_PRINTLN(F("loaded data from saved state"));
  } else {
    DEBUG_PRINTLN(F("no saved state, defining the default state..."));
    storeState();
  }


  DEBUG_PRINTLN(F("init: light sensor ok"));
}

void ChimeLightSensor::storeState() {
  persist->storeLightThreshold(factorThreshold);
}

unsigned short ChimeLightSensor::getLightLevel() {
  return sensor.value();
}

bool ChimeLightSensor::isDark() {
  return getLightLevel() <= darkThreshold;
}

void ChimeLightSensor::publishBluetoothData() {

  {
    ble_light_sensor content;
    content.level = getLightLevel();
    content.isDark = isDark();
    content.min = sensor.envelopeMin();
    content.max = sensor.envelopeMax();  
    this->bluetooth->publishLightSensor(content);
  }

  {
    ble_light_settings content;
    content.threshold = factorThreshold;
    this->bluetooth->publishLightSettings(content);
  }
}

BluetoothListenerAnswer ChimeLightSensor::receivedLightSettings(ble_light_settings content) {

  factorThreshold = content.threshold;
  
  if (factorThreshold < 0) { factorThreshold = 0; }
  else if (factorThreshold > 100) { factorThreshold = 100; }

  storeState();
   
  debugSerial();

  return PROCESSED;

}

Intention ChimeLightSensor::proposeNextMode(enum mode current_mode, unsigned long next_planned_action) {
  
  bool isDarkNow = isDark();
  bool wasDark = previousIsDark;
  previousIsDark = isDarkNow;
  if ( (current_mode == NOTHING) && (!isDarkNow) && (wasDark) ) {
      // light just came back; welcome it!
      DEBUG_PRINTLN(F("welcoming the sun ;-)"));
      return Intention { WELCOME_SUN,  millis() };
  }
  return Intention { current_mode, next_planned_action };
  
}
