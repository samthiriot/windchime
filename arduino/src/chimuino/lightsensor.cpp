#include "lightsensor.h"

#include <Arduino.h>
#include <Streaming.h>

#include "debug.h"

const char msg_lightsensor[] PROGMEM  = { "light sensor"};

ChimeLightSensor::ChimeLightSensor(const byte _pin):
              BluetoothInformationProducer(),
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

    // publish these novel values
    publishBluetoothDataSensor();
  }
}

void ChimeLightSensor::debugSerial() {

  #ifdef DEBUG
  Serial << F("light") << PGMSTR(msg_level) << F(" IS ") 
         << _DEC(getLightLevel()) << ' ' 
         << (isDark() ? F("DARK"): F("LIT")) << ','
         << _DEC(factorThreshold) << '>'
         << _DEC(darkThreshold) << F(" in ")
         << '[' << _DEC(sensor.envelopeMin()) << ':' << _DEC(sensor.envelopeMax()) << ']'
         << endl;
  #endif
}

void ChimeLightSensor::setup(Persist* _persist) {

  TRACE_PRINT(message_init); 
  TRACE_PRINTLN(PGMSTR(msg_lightsensor));

  sensor.setup(0, -1); // we assume no light means 0, but have no idea of the max
  sensor.sense();

  persist = _persist; 
  if (persist->hasDataStored()) {
    // there is data persisted ! Let's load it :-)
    factorThreshold = persist->getLightThreshold();
    TRACE_PRINTLN(PGMSTR(msg_loaded_saved_state));
  } else {
    ERROR_PRINTLN(PGMSTR(msg_not_persisted_default_state));
    storeState();
  }

  TRACE_PRINT(PGMSTR(message_init)); 
  TRACE_PRINT(PGMSTR(msg_lightsensor));
  TRACE_PRINTLN(PGMSTR(msg_ok_dot));
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


void ChimeLightSensor::publishBluetoothDataSettings() {
  ble_light_settings content;
  content.threshold = factorThreshold;
  this->bluetooth->publishLightSettings(content);
}

void ChimeLightSensor::publishBluetoothDataSensor() {
  ble_light_sensor content;
  content.level = getLightLevel();
  content.isDark = isDark();
  content.min = sensor.envelopeMin();
  content.max = sensor.envelopeMax();  
  this->bluetooth->publishLightSensor(content);
}

void ChimeLightSensor::publishBluetoothData() {

  publishBluetoothDataSensor();
  publishBluetoothDataSettings();
 
}

BluetoothListenerAnswer ChimeLightSensor::receivedLightSettings(ble_light_settings content) {

  factorThreshold = content.threshold;
  
  if (factorThreshold < 0) { factorThreshold = 0; }
  else if (factorThreshold > 100) { factorThreshold = 100; }

  storeState();
   
  debugSerial();

  return PROCESSED;

}

Intention ChimeLightSensor::proposeNextMode(Intention currentIntention) {
  
  bool isDarkNow = isDark();
  bool wasDark = previousIsDark;
  previousIsDark = isDarkNow;
  if ( (currentIntention.what == NOTHING) && (!isDarkNow) && (wasDark) ) {
      // light just came back; welcome it!
      TRACE_PRINTLN(F("welcoming the sun ;-)"));
      return Intention { WELCOME_SUN,  millis() };
  }
  return currentIntention;
  
}
