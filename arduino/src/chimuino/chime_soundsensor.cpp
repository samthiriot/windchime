#include "chime_soundsensor.h"

#include <Arduino.h>
#include <Streaming.h>

#include "debug.h"

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

  /* TODO sound sensing
   unsigned long startMillis= millis();  // Start of sample window
   unsigned int peakToPeak = 0;   // peak-to-peak level
 
   unsigned int signalMax = 0;
   unsigned int signalMin = 1024;

   const int sampleWindow = 50; // Sample window width in mS (50 mS = 20Hz)
  unsigned int sample;

   // collect data for 50 mS
   while (millis() - startMillis < sampleWindow)
   {
      sample = analogRead(pin);
      if (sample < 1024)  // toss out spurious readings
      {
         if (sample > signalMax)
         {
            signalMax = sample;  // save just the max levels
         }
         else if (sample < signalMin)
         {
            signalMin = sample;  // save just the min levels
         }
      }
   }
   peakToPeak = signalMax - signalMin;  // max - min = peak-peak amplitude
   double volts = (peakToPeak * 5.0) / 1024;  // convert to volts

   DEBUG_PRINTLN(peakToPeak);
*/


  if (sensor.sense()) {         // update the measure of sound  
    
    // the sensor was updated

    // ... update the threshold!
    quietThreshold = sensor.envelopeMin() + int(float(factorThreshold)* float(sensor.envelopeMax() - sensor.envelopeMin())/100.0);

  }
  
}

void ChimeSoundSensor::debugSerial() {
  
  #ifdef DEBUG
  Serial << F("SOUND LEVEL: ") << _DEC(sensor.value()) << ' ' 
         << ( isQuiet() ? F("(quiet)"):F("(noisy)") ) << ' '
         << _DEC(factorThreshold) << '>'
         << _DEC(quietThreshold) << ' '
         << '[' << _DEC(sensor.envelopeMin()) << ':' << _DEC(sensor.envelopeMax()) << ']'
         << endl;
  #endif
 
}

void ChimeSoundSensor::setup(Persist* _persist) {

  TRACE_PRINT(message_init); 
  TRACE_PRINTLN(F("sound sensor..."));

  sensor.setup(-1, -1); // we have no f idea of what are the min and max for such a sensor !
  sensor.sense();

  persist = _persist; 
  if (persist->hasDataStored()) {
    // there is data persisted ! Let's load it :-)
    factorThreshold = persist->getSoundThreshold();
    TRACE_PRINTLN(F("loaded data from saved state"));
  } else {
    ERROR_PRINTLN(F("no saved state, defining the default state..."));
    storeState();
  }
  
  TRACE_PRINT(message_init); 
  TRACE_PRINTLN(F("sound sensor ok"));
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

void ChimeSoundSensor::publishBluetoothData() {

  {
    ble_sound_sensor content;
    content.level = getSoundLevel();
    content.isQuiet = isQuiet();
    content.min = sensor.envelopeMin();
    content.max = sensor.envelopeMax();  
    this->bluetooth->publishSoundSensor(content);
  }

  {
    ble_sound_settings content;
    content.threshold = factorThreshold;
    this->bluetooth->publishSoundSettings(content);
  }
}

BluetoothListenerAnswer ChimeSoundSensor::receivedSoundSettings(ble_sound_settings content) {

  factorThreshold = content.threshold;
  
  if (factorThreshold < 0) { factorThreshold = 0; }
  else if (factorThreshold > 100) { factorThreshold = 100; }

  storeState();
   
  debugSerial();

  return PROCESSED;

}




