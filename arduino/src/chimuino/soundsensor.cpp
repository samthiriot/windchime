#include "soundsensor.h"

#include <Arduino.h>
#include <Streaming.h>

#include "debug.h"

const char msg_soundsensor[] PROGMEM  = { "sound sensor"};
const char msg_quiet[] PROGMEM  =       { "quiet"};
const char msg_noisy[] PROGMEM  =       { "noisy"};


SoundLowPassFilterSensorWithMinMax::SoundLowPassFilterSensorWithMinMax(
                                  const float _ETA, const byte _pin, const unsigned int _period, 
                                  const float _ETAslow, const float _ETAquick
                                  ):
                                  LowPassFilterSensorWithMinMax(_ETA, _pin, _period, _ETAslow, _ETAquick) {
                                    
}


bool SoundLowPassFilterSensorWithMinMax::sense() {
  
  const unsigned long startMillis = millis();
  if (startMillis - lastReading < period) {
    // last reading is too recent; don't measure again
    return false;
  }

  // measure the value 
  const uint8_t sampleWindow = 50; // Sample window width in mS (50 mS = 20Hz)
  uint8_t peakToPeak = 0;   // peak-to-peak level  
  uint8_t signalMax = 0;
  uint8_t signalMin = 1024;
  uint8_t sample;

  // collect data for 50 ms
  while (millis() - startMillis <= sampleWindow) {
    sample = analogRead(pin);
    if (sample < 1024)  // toss out spurious readings
    {
       if (sample > signalMax) {
          signalMax = sample;  
       }
       else if (sample < signalMin) {
          signalMin = sample;  
       }
    }
  }
  peakToPeak = signalMax - signalMin;  // max - min = peak-peak amplitude
  //double volts = (peakToPeak * 5.0) / 1024;  // convert to volts

  // let's read
  pastvalue = update_value(peakToPeak, pastvalue, ETA);
  lastReading = millis();
  
  // smoothed value
  adaptMinMax(pastvalue);
  
  return true;
}


                                  
ChimeSoundSensor::ChimeSoundSensor(const byte _pin):
              BluetoothInformationProducer(),
              sensor(CHIME_SOUNDSENSOR_MEASURES_ETA, _pin, 
                    CHIME_SOUNDSENSOR_MEASURES_FREQUENCY, // read every 100ms
                    CHIME_SOUNDSENSOR_ENVELOPE_ETA_SLOW, CHIME_SOUNDSENSOR_ENVELOPE_ETA_QUICK
                    )  
              {

  // save params
  pin = _pin;

}

void ChimeSoundSensor::perceive() {


  if (sensor.sense()) {         // update the measure of sound  
    
    // the sensor was updated

    // ... update the threshold!
    quietThreshold = sensor.envelopeMin() + uint8_t(float(factorThreshold)* float(sensor.envelopeMax() - sensor.envelopeMin())/100.0);

    // update published data
    publishBluetoothDataSensor();
  }
  
}

void ChimeSoundSensor::debugSerial() {
  
  #ifdef DEBUG
  Serial << F("sound") << PGMSTR(msg_level) << ' ' << _DEC(sensor.value()) << ' ' 
         << ( isQuiet() ? PGMSTR(msg_quiet):PGMSTR(msg_noisy) ) << ' '
         << _DEC(factorThreshold) << '>'
         << _DEC(quietThreshold) << ' '
         << '[' << _DEC(sensor.envelopeMin()) << ':' << _DEC(sensor.envelopeMax()) << ']'
         << endl;
  #endif
 
}

void ChimeSoundSensor::setup(Persist* _persist) {

  TRACE_PRINT(message_init); 
  TRACE_PRINTLN(PGMSTR(msg_soundsensor));

  sensor.setup(-1, -1); // we have no f idea of what are the min and max for such a sensor !
  sensor.sense();

  persist = _persist; 
  if (persist->hasDataStored()) {
    // there is data persisted ! Let's load it :-)
    factorThreshold = persist->getSoundThreshold();
    TRACE_PRINTLN(PGMSTR(msg_loaded_saved_state));
  } else {
    ERROR_PRINTLN(PGMSTR(msg_not_persisted_default_state));
    storeState();
  };
  
  DEBUG_PRINT(PGMSTR(message_init)); 
  DEBUG_PRINT(PGMSTR(msg_soundsensor));
  DEBUG_PRINTLN(PGMSTR(msg_ok_dot));
}


void ChimeSoundSensor::storeState() {
  persist->storeSoundThreshold(factorThreshold);
}

unsigned short ChimeSoundSensor::measureSoundLevel() {
  return sensor.value();
}

unsigned short ChimeSoundSensor::getSoundLevel() {
  unsigned short raw = measureSoundLevel();
  return raw;
}

bool ChimeSoundSensor::isQuiet() {
  return getSoundLevel() <= quietThreshold;
}

void ChimeSoundSensor::publishBluetoothDataSettings() {
   ble_sound_settings content;
  content.threshold = factorThreshold;
  this->bluetooth->publishSoundSettings(content);
}
void ChimeSoundSensor::publishBluetoothDataSensor() {
  ble_sound_sensor content;
  content.level = getSoundLevel();
  content.isQuiet = isQuiet();
  content.min = sensor.envelopeMin();
  content.max = sensor.envelopeMax();  
  this->bluetooth->publishSoundSensor(content);
}

void ChimeSoundSensor::publishBluetoothData() {

  publishBluetoothDataSettings();
  publishBluetoothDataSensor();
}

BluetoothListenerAnswer ChimeSoundSensor::receivedSoundSettings(ble_sound_settings content) {

  factorThreshold = content.threshold;
  
  if (factorThreshold < 0) { factorThreshold = 0; }
  else if (factorThreshold > 100) { factorThreshold = 100; }

  storeState();
   
  debugSerial();

  return PROCESSED;

}
