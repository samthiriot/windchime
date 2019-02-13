
// hardware note
// * see http://www.martyncurrey.com/hm-10-bluetooth-4ble-modules/ 
// * http://blog.blecentral.com/2015/05/05/hm-10-peripheral/

#include "chime_bluetooth.h"

#include <Streaming.h>

#include "debug.h"

// special characters
#define CR 13
#define LR 10

#include <Arduino.h>

ChimeBluetooth* ChimeBluetooth::singleton = NULL;

// converter between uint16 and bytes
union ble_uint16_bytes {
  uint16_t value;
  uint8_t bytes[sizeof(uint16_t)];
};

// converter between uint32 and bytes
union ble_uint32_bytes {
  uint32_t value;
  uint8_t bytes[sizeof(uint32_t)];
};

// convert between float and bytes
union ble_float_bytes {
  float value;
  uint8_t bytes[sizeof(float)];
};

// convert between datetime and bytes
union ble_datetime_bytes {
  ble_datetime data;
  uint8_t bytes[sizeof(ble_datetime)];
};

// converter between alarm and bytes
union ble_alarm_bytes {
  ble_alarm data;
  uint8_t bytes[sizeof(ble_alarm)];
};

// converter between ambiance and bytes
union ble_ambiance_bytes {
  ble_ambiance data;
  uint8_t bytes[sizeof(ble_ambiance)];
};

// converter between light level and bytes
union ble_light_sensor_bytes {
  ble_light_sensor data;
  uint8_t bytes[sizeof(ble_light_sensor)];
};

// converter between light settings and bytes
union ble_light_settings_bytes {
  ble_light_settings data;
  uint8_t bytes[sizeof(ble_light_settings)];
};

// converter between light level and bytes
union ble_sound_sensor_bytes {
  ble_sound_sensor data;
  uint8_t bytes[sizeof(ble_sound_sensor)];
};

// converter between light settings and bytes
union ble_sound_settings_bytes {
  ble_sound_settings data;
  uint8_t bytes[sizeof(ble_sound_settings)];
};

ChimeBluetooth::ChimeBluetooth(unsigned short _pinTXD, unsigned short _pinRXD, 
                               unsigned short _pinMode, unsigned short _pinCTS, unsigned short _pinRTS):
      bluefruitSS(_pinTXD, _pinRXD),
      ble(bluefruitSS, _pinMode, _pinCTS, _pinRTS),
      gatt(ble) {

  // store this instance as the singleton instance
  ChimeBluetooth::singleton = this;
  // TODO detect redefinition of singleton
  
}


void ChimeBluetooth::setup() {

  // initialize the BLE access (and detect HW problems)
  DEBUG_PRINT(F("init / bluetooth: connecting hardware... "));
  if ( !ble.begin(BLE_VERBOSE_MODE) ) {
      DEBUG_PRINTLN(F("ERROR Couldn't find Bluefruit, make sure it's in command mode & check wiring?"));
  } else {
    DEBUG_PRINTLN( F("OK.") );
  }
  
  #ifdef BLE_FACTORYRESET_ENABLE 
  /* Perform a factory reset to make sure everything is in a known state */
  DEBUG_PRINT(F("init / bluetooth: factory reset... "));
  if ( !ble.factoryReset() ){
    DEBUG_PRINTLN(F("ERROR"));
  } else {
    DEBUG_PRINTLN( F("OK.") );
  }
  #endif

  /* Disable command echo from Bluefruit */
  ble.echo(false);

  DEBUG_PRINT(F("init / bluetooth: setting name... "));
  if (! ble.sendCommandCheckOK(F( "AT+GAPDEVNAME=Chimuino2" ))) {
    DEBUG_PRINTLN(F("ERROR"));
  } else {
    DEBUG_PRINTLN( F("OK.") );
  }

  DEBUG_PRINTLN(F("init / bluetooth: creating services... "));
  // SET the services and characteristics
  setup_service_chimuino();
  setup_service_sensing();
   
  /* Reset the device for the new service setting changes to take effect */
  DEBUG_PRINTLN(F("init / bluetooth: reset to apply changes... "));
  ble.reset();
  delay(100);
  

  DEBUG_PRINTLN(F("init / bluetooth: installing callbacks... "));
  // set callbacks to be called when...
  // ... some central device connects,
  ble.setConnectCallback(ChimeBluetooth::reactCentralConnectedStatic);
  // ... disconnects,
  ble.setDisconnectCallback(ChimeBluetooth::reactCentralDisconnectedStatic);
  // ... or when an attribute value changed!
  ble.setBleGattRxCallback(bleCharCurrentTime,    ChimeBluetooth::reactCharacteristicReceivedStatic);
  ble.setBleGattRxCallback(bleCharAlarm1,         ChimeBluetooth::reactCharacteristicReceivedStatic);
  ble.setBleGattRxCallback(bleCharAlarm2,         ChimeBluetooth::reactCharacteristicReceivedStatic);
  ble.setBleGattRxCallback(bleCharAmbiance,       ChimeBluetooth::reactCharacteristicReceivedStatic);
  ble.setBleGattRxCallback(bleCharLightSettings,  ChimeBluetooth::reactCharacteristicReceivedStatic);

  DEBUG_PRINTLN(F("init: bluetooth ok"));
}


void ChimeBluetooth::setup_service_sensing() {
   
  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.service.environmental_sensing.xml
  Serial.println(F("Adding the Sensing Service definition: "));
  bleServiceSensingId = gatt.addService(BLE_GATT_SERVICE_SENSING);
  if (bleServiceSensingId == 0) {
    Serial.println(F("Could not add sensing service"));
  } else {
    setup_char_temperature();
    setup_char_light_sensor();
    setup_char_light_settings();
    setup_char_sound_sensor();
    setup_char_sound_settings();
    
    // TODO sound
    // TODO ...
    
  }
}

void ChimeBluetooth::setup_char_temperature() {
  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.characteristic.temperature.xml
    
    GattPresentationFormat bleTemperatureFormat = { 
      // format
      GATT_PRESENT_FORMAT_SINT16, 
      // exponent 
      2,
      // unit
      GATT_PRESENT_UNIT_THERMODYNAMIC_TEMPERATURE_DEGREE_CELSIUS,
      // name space, desc
      NULL, NULL
      };
    
    bleCharTemperature = gatt.addCharacteristic(
        // UUID
        BLE_GATT_CHAR_TEMPERATURE, 
        // properties
        GATT_CHARS_PROPERTIES_READ | GATT_CHARS_PROPERTIES_NOTIFY, 
        // data size min and max
        sizeof(int16_t), sizeof(int16_t), 
        // datatype
        BLE_DATATYPE_BYTEARRAY, //BLE_DATATYPE_INTEGER
        // description
        NULL, 
        // present Format 
        NULL//&bleTemperatureFormat // NULL // 
        );
    
    if (bleCharTemperature == 0) {
      DEBUG_PRINTLN(F("Could not add Temperature characteristic"));
    } 
}

void ChimeBluetooth::setup_char_light_sensor() {
  
    DEBUG_PRINTLN(F("Adding the characteristic light sensor"));
 
    // GATT_CHARS_PROPERTIES_WRITE
    bleCharLightSensor = gatt.addCharacteristic(
      // format
      BLE_GATT_CHAR_LIGHT_SENSOR,
      // properties
      GATT_CHARS_PROPERTIES_READ | GATT_CHARS_PROPERTIES_NOTIFY, 
      // data size min, max, 
      sizeof(ble_light_sensor_bytes), sizeof(ble_light_sensor_bytes), 
      // present format
      BLE_DATATYPE_BYTEARRAY
      );
    if (bleCharAmbiance == 0) {
      DEBUG_PRINTLN(F("Could not add characteristic light sensor"));
    } 
}

void ChimeBluetooth::setup_char_light_settings() {
  
    DEBUG_PRINTLN(F("Adding the characteristic light settings"));
 
    // GATT_CHARS_PROPERTIES_WRITE
    bleCharLightSettings = gatt.addCharacteristic(
      // format
      BLE_GATT_CHAR_LIGHT_SETTINGS,
      // properties
      GATT_CHARS_PROPERTIES_READ | GATT_CHARS_PROPERTIES_WRITE, 
      // data size min, max, 
      sizeof(ble_light_settings_bytes), sizeof(ble_light_settings_bytes), 
      // present format
      BLE_DATATYPE_BYTEARRAY
      );
    if (bleCharLightSettings == 0) {
      DEBUG_PRINTLN(F("Could not add characteristic light settings"));
    } 
}

void ChimeBluetooth::setup_char_sound_sensor() {
  
    DEBUG_PRINTLN(F("Adding the characteristic sound sensor"));
 
    // GATT_CHARS_PROPERTIES_WRITE
    bleCharSoundSensor = gatt.addCharacteristic(
      // format
      BLE_GATT_CHAR_SOUND_SENSOR,
      // properties
      GATT_CHARS_PROPERTIES_READ | GATT_CHARS_PROPERTIES_NOTIFY, 
      // data size min, max, 
      sizeof(ble_sound_sensor_bytes), sizeof(ble_sound_sensor_bytes), 
      // present format
      BLE_DATATYPE_BYTEARRAY
      );
    if (bleCharSoundSensor == 0) {
      DEBUG_PRINTLN(F("Could not add characteristic sound sensor"));
    } 
}

void ChimeBluetooth::setup_char_sound_settings() {
  
    DEBUG_PRINTLN(F("Adding the characteristic sound settings"));
 
    // GATT_CHARS_PROPERTIES_WRITE
    bleCharSoundSettings = gatt.addCharacteristic(
      // format
      BLE_GATT_CHAR_LIGHT_SETTINGS,
      // properties
      GATT_CHARS_PROPERTIES_READ | GATT_CHARS_PROPERTIES_WRITE, 
      // data size min, max, 
      sizeof(ble_sound_settings_bytes), sizeof(ble_sound_settings_bytes), 
      // present format
      BLE_DATATYPE_BYTEARRAY
      );
    if (bleCharSoundSettings == 0) {
      DEBUG_PRINTLN(F("Could not add characteristic sound settings"));
    } 
}

void ChimeBluetooth::setup_service_chimuino() {


  /* Add the Heart Rate Service definition */
  /* Service ID should be 1 */
  DEBUG_PRINTLN(F("Adding the Chuimuino Service definition: "));
  bleServiceChimuinoId = gatt.addService(BLE_GATT_SERVICE_CHIMUINO);
  if (bleServiceChimuinoId == 0) {
    DEBUG_PRINTLN(F("Could not add Chuimuino service"));
  } else {

    setup_attribute_current_time();
    setup_attribute_alarm1();
    setup_attribute_alarm2();
    setup_attribute_ambiance();
    setup_attribute_temperature1();
    setup_attribute_temperature2();
    setup_attribute_uptime();

    // TODO ...
    
  }

}

void ChimeBluetooth::setup_attribute_current_time() {
  
    /* Add the Temperature Measurement characteristic which is composed of
     * 1 byte flags + 4 float */
    /* Chars ID for Measurement should be 1 */
    DEBUG_PRINTLN(F("Adding the current time characteristic: "));
    
    // GATT_CHARS_PROPERTIES_WRITE
    bleCharCurrentTime = gatt.addCharacteristic(
      // format
      BLE_GATT_CHAR_TIME, 
      // properties
      GATT_CHARS_PROPERTIES_READ | GATT_CHARS_PROPERTIES_WRITE, 
      // data size min, max, 
      sizeof(ble_datetime), sizeof(ble_datetime), 
      // present format
      BLE_DATATYPE_BYTEARRAY
      );
    if (bleCharCurrentTime == 0) {
      DEBUG_PRINTLN(F("Could not add current time characteristic"));
    } 
  
}

void ChimeBluetooth::setup_attribute_alarm1() {
  
    DEBUG_PRINTLN(F("Adding the alarm1 characteristic: "));
 
    // GATT_CHARS_PROPERTIES_WRITE
    bleCharAlarm1 = gatt.addCharacteristic(
      // format
      BLE_GATT_CHAR_ALARM1, 
      // properties
      GATT_CHARS_PROPERTIES_READ | GATT_CHARS_PROPERTIES_WRITE, 
      // data size min, max, 
      sizeof(ble_alarm), sizeof(ble_alarm), 
      // present format
      BLE_DATATYPE_BYTEARRAY
      );
    if (bleCharAlarm1 == 0) {
      DEBUG_PRINTLN(F("Could not add alarm1 characteristic"));
    } 
}


void ChimeBluetooth::setup_attribute_alarm2() {
  
    DEBUG_PRINTLN(F("Adding the alarm2 characteristic: "));
 
    // GATT_CHARS_PROPERTIES_WRITE
    bleCharAlarm2 = gatt.addCharacteristic(
      // format
      BLE_GATT_CHAR_ALARM2, 
      // properties
      GATT_CHARS_PROPERTIES_READ | GATT_CHARS_PROPERTIES_WRITE, 
      // data size min, max, 
      sizeof(ble_alarm), sizeof(ble_alarm), 
      // present format
      BLE_DATATYPE_BYTEARRAY
      );
    if (bleCharAlarm2 == 0) {
      DEBUG_PRINTLN(F("Could not add alarm2 characteristic"));
    } 
}

void ChimeBluetooth::setup_attribute_ambiance() {
  
    DEBUG_PRINTLN(F("Adding the ambiance characteristic"));
 
    // GATT_CHARS_PROPERTIES_WRITE
    bleCharAmbiance = gatt.addCharacteristic(
      // format
      BLE_GATT_CHAR_AMBIANCE, 
      // properties
      GATT_CHARS_PROPERTIES_READ | GATT_CHARS_PROPERTIES_WRITE, 
      // data size min, max, 
      sizeof(ble_ambiance_bytes), sizeof(ble_ambiance_bytes), 
      // present format
      BLE_DATATYPE_BYTEARRAY
      );
    if (bleCharAmbiance == 0) {
      DEBUG_PRINTLN(F("Could not add ambiance characteristic"));
    } 
}

void ChimeBluetooth::setup_attribute_temperature1() {
      
    DEBUG_PRINTLN(F("Adding the characteristic temperature1"));
 
    // GATT_CHARS_PROPERTIES_WRITE
    bleCharTemperature1 = gatt.addCharacteristic(
      // format
      BLE_GATT_CHAR_TEMPERATURE1, 
      // properties
      GATT_CHARS_PROPERTIES_READ | GATT_CHARS_PROPERTIES_NOTIFY, 
      // data size min, max, 
      sizeof(ble_float_bytes), sizeof(ble_float_bytes), 
      // present format
      BLE_DATATYPE_BYTEARRAY
      );
    if (bleCharTemperature1 == 0) {
      DEBUG_PRINTLN(F("Could not add temperature2 characteristic"));
    } 
}

void ChimeBluetooth::setup_attribute_temperature2() {
      
    DEBUG_PRINTLN(F("Adding the characteristic temperature2"));
 
    // GATT_CHARS_PROPERTIES_WRITE
    bleCharTemperature2 = gatt.addCharacteristic(
      // format
      BLE_GATT_CHAR_TEMPERATURE2, 
      // properties
      GATT_CHARS_PROPERTIES_READ | GATT_CHARS_PROPERTIES_NOTIFY, 
      // data size min, max, 
      sizeof(ble_float_bytes), sizeof(ble_float_bytes), 
      // present format
      BLE_DATATYPE_BYTEARRAY
      );
    if (bleCharTemperature2 == 0) {
      DEBUG_PRINTLN(F("Could not add temperature2 characteristic"));
    } 
}

void ChimeBluetooth::setup_attribute_uptime() {
      
    DEBUG_PRINTLN(F("Adding the characteristic uptime"));
 
    // GATT_CHARS_PROPERTIES_WRITE
    bleCharUptime = gatt.addCharacteristic(
      // format
      BLE_GATT_CHAR_UPTIME, 
      // properties
      GATT_CHARS_PROPERTIES_READ, 
      // data size min, max, 
      sizeof(ble_uint32_bytes), sizeof(ble_uint32_bytes), 
      // present format
      BLE_DATATYPE_BYTEARRAY
      );
    if (bleCharUptime == 0) {
      DEBUG_PRINTLN(F("Could not add uptime characteristic"));
    } 
}

void ChimeBluetooth::decodeCurrentDateTime(uint8_t data[], uint16_t len) {

   // ensure the length is ok
   if (len != sizeof(ble_datetime_bytes)) {
      DEBUG_PRINT(F("WRONG LENGTH: "));
      DEBUG_PRINTLN(len);
      return;
   } 
   
   // decode from bytes
   ble_datetime_bytes currentTime;
   memcpy(&currentTime, data, len);

  // warn listeners
  for (int i=0; i<listeners_count; i++) {
    if (NOT_CONCERNED != listeners[i]->receivedCurrentDateTime(currentTime.data)) { break; };
  }
}

void ChimeBluetooth::decodeAlarm1(uint8_t data[], uint16_t len) {

  // ensure the length is ok
  if (len != sizeof(ble_alarm_bytes)) {
    DEBUG_PRINT(F("WRONG LENGTH: "));
    DEBUG_PRINTLN(len);
    return;
  } 

  // decode from bytes
  ble_alarm_bytes received_alarm;
  memcpy(&received_alarm, data, len);
  
  // warn listeners
  for (int i=0; i<listeners_count; i++) {
    if (NOT_CONCERNED != listeners[i]->receivedAlarm1(received_alarm.data)) { break; };
  }
}

void ChimeBluetooth::decodeAlarm2(uint8_t data[], uint16_t len) {

  // ensure the length is ok
  if (len != sizeof(ble_alarm_bytes)) {
    DEBUG_PRINT(F("WRONG LENGTH: "));
    DEBUG_PRINTLN(len);
    return;
  } 

  // decode from bytes
  ble_alarm_bytes received_alarm;
  memcpy(&received_alarm, data, len);
  
  // warn listeners
  for (int i=0; i<listeners_count; i++) {
    if (NOT_CONCERNED != listeners[i]->receivedAlarm2(received_alarm.data)) { break; };
  }
}


void ChimeBluetooth::decodeAmbiance(uint8_t data[], uint16_t len) {
  
  // ensure the length is ok
  if (len != sizeof(ble_ambiance_bytes)) {
    DEBUG_PRINT(F("WRONG LENGTH: "));
    DEBUG_PRINTLN(len);
    return;
  } 

  // decode from bytes
  ble_ambiance_bytes received_ambiance;
  memcpy(&received_ambiance, data, len);
  
  // warn listeners
  for (int i=0; i<listeners_count; i++) {
    if (NOT_CONCERNED != listeners[i]->receivedAmbiance(received_ambiance.data)) { break; };
  }
}

void ChimeBluetooth::decodeLightSettings(uint8_t data[], uint16_t len) {
  
  // ensure the length is ok
  if (len != sizeof(ble_light_settings_bytes)) {
    DEBUG_PRINT(F("WRONG LENGTH: "));
    DEBUG_PRINTLN(len);
    return;
  } 

  // decode from bytes
  ble_light_settings_bytes received_settings;
  memcpy(&received_settings, data, len);
  
  // warn listeners
  for (int i=0; i<listeners_count; i++) {
    if (NOT_CONCERNED != listeners[i]->receivedLightSettings(received_settings.data)) { break; };
  }
}

void ChimeBluetooth::decodeSoundSettings(uint8_t data[], uint16_t len) {
  
  // ensure the length is ok
  if (len != sizeof(ble_sound_settings_bytes)) {
    DEBUG_PRINT(F("WRONG LENGTH: "));
    DEBUG_PRINTLN(len);
    return;
  } 

  // decode from bytes
  ble_sound_settings_bytes received_settings;
  memcpy(&received_settings, data, len);
  
  // warn listeners
  for (int i=0; i<listeners_count; i++) {
    if (NOT_CONCERNED != listeners[i]->receivedSoundSettings(received_settings.data)) { break; };
  }
}


void ChimeBluetooth::reactCharacteristicReceived(int32_t chars_id, uint8_t data[], uint16_t len) {

  if (chars_id == bleCharCurrentTime) {
    decodeCurrentDateTime(data, len);
  } else if (chars_id == bleCharAlarm1) {
    decodeAlarm1(data, len);
  } else if (chars_id == bleCharAlarm2) {
    decodeAlarm2(data, len);
  } else if (chars_id == bleCharAmbiance) {
    decodeAmbiance(data, len);
  } else if (chars_id == bleCharLightSettings) {
    decodeLightSettings(data, len); 
  } else if (chars_id == bleCharSoundSettings) {
    decodeSoundSettings(data, len); 
  } else {
    DEBUG_PRINT(F("ERROR: UNKNOWN BLUETOOTH CHARACTERISTIC "));
    DEBUG_PRINTLN(chars_id);
  }
}

void ChimeBluetooth::reactCharacteristicReceivedStatic(int32_t chars_id, uint8_t data[], uint16_t len) {
  singleton->reactCharacteristicReceivedStatic(chars_id, data, len);
};

void ChimeBluetooth::reactCentralConnected() {
  DEBUG_PRINT(F("Bluetooth: central connected "));
 
  if (ble.sendCommandCheckOK("AT+BLEGETPEERADDR")) {
    DEBUG_PRINTLN(ble.buffer);
  } else {
    DEBUG_PRINTLN('?');
  }
};

void ChimeBluetooth::reactCentralConnectedStatic() {
    singleton->reactCentralConnected();
};

void ChimeBluetooth::reactCentralDisconnected()  {
  DEBUG_PRINTLN(F("Bluetooth: central disconnected"));
};

void ChimeBluetooth::reactCentralDisconnectedStatic()  {
  singleton->reactCentralDisconnected();
};

void ChimeBluetooth::addBluetoothListener(BluetoothUser* listener) {
  if (listeners_count >= BLUETOOTH_MAX_LISTENERS) {
    // cannot add another listener, all the slots are taken :-(
    // TODO warn
    return;
  }
  listeners[listeners_count++] = listener;
}

void ChimeBluetooth::publishAmbiance(ble_ambiance ambiance) {

  // TODO
  ble_ambiance_bytes content;
  content.data = ambiance;
  
  DEBUG_PRINTLN(F("Bluetooth: publishing ambiance"));
  
  // update the corresponding attribute value
  gatt.setChar(bleCharAmbiance, content.bytes, sizeof(content.bytes));

}

void ChimeBluetooth::publishAlarm1(ble_alarm alarm) {

  // encore as bytes
  ble_alarm_bytes content;
  content.data = alarm;
  
  DEBUG_PRINTLN(F("Bluetooth: publishing alarm1"));
  
  // update the corresponding attribute value
  gatt.setChar(bleCharAlarm1, content.bytes, sizeof(content.bytes));

}

void ChimeBluetooth::publishAlarm2(ble_alarm alarm) {

  // encore as bytes
  ble_alarm_bytes content;
  content.data = alarm;
  
  DEBUG_PRINTLN(F("Bluetooth: publishing alarm2"));
  
  // update the corresponding attribute value
  gatt.setChar(bleCharAlarm2, content.bytes, sizeof(content.bytes));

}

void ChimeBluetooth::publishTemperature1(float temp) {

  // encore as bytes
  ble_float_bytes content;
  content.value = temp;
  
  DEBUG_PRINTLN(F("Bluetooth: publishing temperature1"));
  
  // update the corresponding attribute value
  gatt.setChar(bleCharTemperature1, content.bytes, sizeof(content.bytes));

}

void ChimeBluetooth::publishTemperature2(float temp) {

  // encore as bytes
  ble_float_bytes content;
  content.value = temp;
  
  DEBUG_PRINTLN(F("Bluetooth: publishing temperature2"));
  
  // update the corresponding attribute value
  gatt.setChar(bleCharTemperature2, content.bytes, sizeof(content.bytes));

}


void ChimeBluetooth::publishUptime(uint32_t uptime_min) {

  // encore as bytes
  ble_uint32_bytes content;
  content.value = uptime_min;
  
  DEBUG_PRINTLN(F("Bluetooth: publishing uptime"));
  
  // update the corresponding attribute value
  gatt.setChar(bleCharUptime, content.bytes, sizeof(content.bytes));

}


void ChimeBluetooth::publishLightSensor(ble_light_sensor light_sensor) {

  // encore as bytes
  ble_light_sensor_bytes content;
  content.data = light_sensor;
  
  DEBUG_PRINTLN(F("Bluetooth: publishing light sensor"));
  
  // update the corresponding attribute value
  gatt.setChar(bleCharLightSensor, content.bytes, sizeof(content.bytes));

}


void ChimeBluetooth::publishLightSettings(ble_light_settings settings) {

  // encore as bytes
  ble_light_settings_bytes content;
  content.data = settings;
  
  DEBUG_PRINTLN(F("Bluetooth: publishing light settings"));
  
  // update the corresponding attribute value
  gatt.setChar(bleCharLightSettings, content.bytes, sizeof(content.bytes));

}



void ChimeBluetooth::publishSoundSensor(ble_sound_sensor sound_sensor) {

  // encore as bytes
  ble_sound_sensor_bytes content;
  content.data = sound_sensor;
  
  DEBUG_PRINTLN(F("Bluetooth: publishing sound sensor"));
  
  // update the corresponding attribute value
  gatt.setChar(bleCharSoundSensor, content.bytes, sizeof(content.bytes));

}


void ChimeBluetooth::publishSoundSettings(ble_sound_settings settings) {

  // encore as bytes
  ble_sound_settings_bytes content;
  content.data = settings;
  
  DEBUG_PRINTLN(F("Bluetooth: publishing sound settings"));
  
  // update the corresponding attribute value
  gatt.setChar(bleCharSoundSettings, content.bytes, sizeof(content.bytes));

}


/**
 * Reads whatever available from bluetooth
 */
void ChimeBluetooth::readAndReact() {

  // check if anything new on the side of BLE?
  ble.update(200);

  
}





