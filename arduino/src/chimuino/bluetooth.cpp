
// hardware note
// * see http://www.martyncurrey.com/hm-10-bluetooth-4ble-modules/ 
// * http://blog.blecentral.com/2015/05/05/hm-10-peripheral/

#include "bluetooth.h"

#include <Streaming.h>

#include "debug.h"

#include "chime.h"
#include "clock.h"
#include "alarm.h"
#include "soundsensor.h"
#include "lightsensor.h"
#include "ambiance.h"

// messages for debug
// factorized to save memory space

  const char message_ble_light_sensor [] PROGMEM = "light sensor";
  const char message_ble_light_settings [] PROGMEM = "light settings";
  const char message_ble_sound_sensor [] PROGMEM = "sound sensor";
  const char message_ble_sound_settings [] PROGMEM = "sound settings";
  const char message_ble_ambiance [] PROGMEM = "ambiance";
  const char message_ble_actions [] PROGMEM = "actions";
  
  const char message_ble_sensing [] PROGMEM = "sensing";
  const char message_ble_temperature [] PROGMEM = "temperature";  
  const char message_ble_current_time [] PROGMEM = "current time";
  const char message_ble_alarm [] PROGMEM = "alarm";
  const char message_ble_uptime [] PROGMEM = "uptime";

  const char message_ble_init_bluetooth [] PROGMEM = "init/bluetooth: ";
  const char message_ble_bluetooth [] PROGMEM = "bluetooth: ";
  const char message_ble_bluetooth_publishing [] PROGMEM = "bluetooth: publishing ";
  const char message_ble_adding_char [] PROGMEM = "init/bluetooth: adding characteristic ";

  const char message_ble_error [] PROGMEM = "error :-(";
  const char message_ble_error_creating_char [] PROGMEM = "bluetooth: error when creating characteristic ";
  const char message_ble_error_creating_service [] PROGMEM = "bluetooth: error when creating service ";
  const char message_wrong_size [] PROGMEM = "bluetooth ERROR: wrong length ";

#include <Arduino.h>

// callbacks have to be static; they redirect the query to the singleton instance

ChimeBluetooth* ble_singleton = NULL;
void reactCharacteristicReceivedStatic(int32_t chars_id, uint8_t data[], uint16_t len) {
  ble_singleton->reactCharacteristicReceived(chars_id, data, len);
};

void reactCentralConnectedStatic() {
    ble_singleton->reactCentralConnected();
};
void reactCentralDisconnectedStatic()  {
  ble_singleton->reactCentralDisconnected();
};

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

// converter between actions and bytes
union ble_actions_bytes {
  ble_actions data;
  uint8_t bytes[sizeof(ble_actions)];
};

ChimeBluetooth::ChimeBluetooth(unsigned short _pinTXD, unsigned short _pinRXD, 
                               unsigned short _pinMode, unsigned short _pinCTS, unsigned short _pinRTS,
                               unsigned short _pinButtonConnect, unsigned short _pinButtonSwitch) {

  // store the pins
  pinButtonConnect = _pinButtonConnect;
  pinButtonSwitch = _pinButtonSwitch;
  
  // store this instance as the singleton instance
  ble_singleton = this;
  
}


void ChimeBluetooth::setup() {

  // button pin is INPUT and has internal pullup active
  pinMode(pinButtonConnect,INPUT_PULLUP);
  pinMode(pinButtonSwitch,INPUT_PULLUP);


  // initialize the BLE access (and detect HW problems)
  DEBUG_PRINT(PGMSTR(message_ble_init_bluetooth));
  DEBUG_PRINT(F("connecting hardware... "));
  if ( !ble.begin(BLE_VERBOSE_MODE) ) {
      ERROR_PRINT(PGMSTR(msg_error_semicol));
      ERROR_PRINTLN(F("BLE dongle not found")); // , make sure it's in command mode & check wiring?
  } else {
    DEBUG_PRINTLN(PGMSTR(msg_ok_dot));
  }
  
  #ifdef BLE_FACTORYRESET_ENABLE 
  /* Perform a factory reset to make sure everything is in a known state */
  DEBUG_PRINT(PGMSTR(message_ble_init_bluetooth));
  DEBUG_PRINTLN(F("factory reset... "));
  if ( !ble.factoryReset() ){
    ERROR_PRINTLN(PGMSTR(message_ble_error));
  } else {
    DEBUG_PRINTLN(PGMSTR(msg_ok_dot));
  }
  #endif

  /* Disable command echo from Bluefruit */
  ble.echo(false);

  TRACE_PRINT(PGMSTR(message_ble_init_bluetooth));
  TRACE_PRINT(F("setting name... "));
  if (! ble.sendCommandCheckOK(F( "AT+GAPDEVNAME=Chimuino2" ))) {
    TRACE_PRINTLN(PGMSTR(message_ble_error));
  } else {
    TRACE_PRINTLN(PGMSTR(msg_ok_dot));
  }

  DEBUG_PRINT(PGMSTR(message_ble_init_bluetooth));
  DEBUG_PRINTLN(F("creating services... "));
  // SET the services and characteristics
  setup_service_chimuino();
  setup_service_sensing();
   
  /* Reset the device for the new service setting changes to take effect */
  DEBUG_PRINT(PGMSTR(message_ble_init_bluetooth));
  DEBUG_PRINTLN(F("reset to apply changes... "));
  ble.reset();
  delay(200);
  
  TRACE_PRINT(PGMSTR(message_ble_init_bluetooth));
  TRACE_PRINTLN(F("installing callbacks... "));
  // set callbacks to be called when...
  // ... some central device connects,
  ble.setConnectCallback(reactCentralConnectedStatic);
  // ... disconnects,
  ble.setDisconnectCallback(reactCentralDisconnectedStatic);
  // ... or when an attribute value changed!
  ble.setBleGattRxCallback(bleCharCurrentTime,    reactCharacteristicReceivedStatic);
  ble.setBleGattRxCallback(bleCharAlarm1,         reactCharacteristicReceivedStatic);
  ble.setBleGattRxCallback(bleCharAlarm2,         reactCharacteristicReceivedStatic);
  ble.setBleGattRxCallback(bleCharAmbiance,       reactCharacteristicReceivedStatic);
  ble.setBleGattRxCallback(bleCharLightSettings,  reactCharacteristicReceivedStatic);
  ble.setBleGattRxCallback(bleCharSoundSettings,  reactCharacteristicReceivedStatic);
  ble.setBleGattRxCallback(bleCharActions,        reactCharacteristicReceivedStatic);

  DEBUG_PRINT(PGMSTR(message_ble_init_bluetooth));
  DEBUG_PRINTLN(PGMSTR(msg_ok_dot));
}


void ChimeBluetooth::setup_service_sensing() {
   
  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.service.environmental_sensing.xml
  TRACE_PRINT(PGMSTR(message_ble_error_creating_service));
  TRACE_PRINTLN(PGMSTR(message_ble_sensing));
  
  bleServiceSensingId = gatt.addService(BLE_GATT_SERVICE_SENSING);
  if (bleServiceSensingId == 0) {
    ERROR_PRINT(PGMSTR(message_ble_error_creating_service));
    ERROR_PRINTLN(PGMSTR(message_ble_sensing));
  } else {
    setup_char_temperature();
    setup_attribute_temperature1();
    setup_attribute_temperature2();
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
    
    TRACE_PRINT(PGMSTR(message_ble_adding_char));
    TRACE_PRINTLN(PGMSTR(message_ble_temperature));

    /*
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
    */
    
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
      ERROR_PRINT(PGMSTR(message_ble_error_creating_char));
      ERROR_PRINTLN(PGMSTR(message_ble_temperature));
    } 
}

void ChimeBluetooth::setup_attribute_temperature1() {

    TRACE_PRINT(PGMSTR(message_ble_adding_char));
    TRACE_PRINT(PGMSTR(message_ble_temperature));
    TRACE_PRINTLN('1');
 
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
      ERROR_PRINT(PGMSTR(message_ble_error_creating_char));
      ERROR_PRINT(PGMSTR(message_ble_temperature));
      ERROR_PRINTLN('1');
    } 
}

void ChimeBluetooth::setup_attribute_temperature2() {

   TRACE_PRINT(PGMSTR(message_ble_adding_char));
    TRACE_PRINT(PGMSTR(message_ble_temperature));
    TRACE_PRINTLN('1');
 
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
      ERROR_PRINT(PGMSTR(message_ble_error_creating_char));
      ERROR_PRINT(PGMSTR(message_ble_temperature));
      ERROR_PRINTLN('2');
    } 
}
void ChimeBluetooth::setup_char_light_sensor() {

    TRACE_PRINT(PGMSTR(message_ble_adding_char));
    TRACE_PRINTLN(PGMSTR(message_ble_light_sensor));
 
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
    if (bleCharLightSensor == 0) {
      ERROR_PRINT(PGMSTR(message_ble_error_creating_char));
      ERROR_PRINTLN(PGMSTR(message_ble_light_sensor));
    } 
}

void ChimeBluetooth::setup_char_light_settings() {

    TRACE_PRINT(PGMSTR(message_ble_adding_char));
    TRACE_PRINTLN(PGMSTR(message_ble_light_settings));
 
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
      ERROR_PRINT(PGMSTR(message_ble_error_creating_char));
      ERROR_PRINTLN(PGMSTR(message_ble_light_settings));
    } 
}

void ChimeBluetooth::setup_char_sound_sensor() {

  TRACE_PRINT(PGMSTR(message_ble_adding_char));
    TRACE_PRINTLN(PGMSTR(message_ble_sound_sensor));
 
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
      ERROR_PRINT(PGMSTR(message_ble_error_creating_char));
      ERROR_PRINTLN(PGMSTR(message_ble_sound_sensor));
    } 
}

void ChimeBluetooth::setup_char_sound_settings() {

    TRACE_PRINT(PGMSTR(message_ble_adding_char));
    TRACE_PRINTLN(PGMSTR(message_ble_sound_settings));
 
    // GATT_CHARS_PROPERTIES_WRITE
    bleCharSoundSettings = gatt.addCharacteristic(
      // format
      BLE_GATT_CHAR_SOUND_SETTINGS,
      // properties
      GATT_CHARS_PROPERTIES_READ | GATT_CHARS_PROPERTIES_WRITE, 
      // data size min, max, 
      sizeof(ble_sound_settings_bytes), sizeof(ble_sound_settings_bytes), 
      // present format
      BLE_DATATYPE_BYTEARRAY
      );
    if (bleCharSoundSettings == 0) {
      ERROR_PRINT(PGMSTR(message_ble_error_creating_char));
      ERROR_PRINTLN(PGMSTR(message_ble_sound_settings));
    } 
}


void ChimeBluetooth::setup_service_chimuino() {

  TRACE_PRINT(PGMSTR(message_ble_bluetooth_publishing));
  TRACE_PRINTLN(F("service chime"));
  bleServiceChimuinoId = gatt.addService(BLE_GATT_SERVICE_CHIMUINO);
  if (bleServiceChimuinoId == 0) {
      ERROR_PRINT(PGMSTR(message_ble_error_creating_service));
      ERROR_PRINTLN(F("chime"));
  } else {

    setup_attribute_current_time();
    setup_attribute_alarm1();
    setup_attribute_alarm2();
    setup_attribute_ambiance();
    setup_attribute_uptime();
    setup_char_actions();

    // TODO ...
    
  }

}

void ChimeBluetooth::setup_attribute_current_time() {
  
    /* Add the Temperature Measurement characteristic which is composed of
     * 1 byte flags + 4 float */
    /* Chars ID for Measurement should be 1 */
    TRACE_PRINT(PGMSTR(message_ble_adding_char));
    TRACE_PRINTLN(PGMSTR(message_ble_current_time));
    
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
      ERROR_PRINT(PGMSTR(message_ble_error_creating_char));
      ERROR_PRINTLN(PGMSTR(message_ble_current_time));
    } 
  
}

void ChimeBluetooth::setup_attribute_alarm1() {

    TRACE_PRINT(PGMSTR(message_ble_adding_char));
    TRACE_PRINT(PGMSTR(message_ble_alarm));
    TRACE_PRINTLN('1');
 
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
      ERROR_PRINT(PGMSTR(message_ble_error_creating_char));
      ERROR_PRINT(PGMSTR(message_ble_alarm));
      ERROR_PRINTLN('1');
    } 
}


void ChimeBluetooth::setup_attribute_alarm2() {

    TRACE_PRINT(PGMSTR(message_ble_adding_char));
    TRACE_PRINT(PGMSTR(message_ble_alarm));
    TRACE_PRINTLN('2');
 
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
      ERROR_PRINT(PGMSTR(message_ble_error_creating_char));
      ERROR_PRINT(PGMSTR(message_ble_alarm));
      ERROR_PRINTLN('2');
    } 
}

void ChimeBluetooth::setup_attribute_ambiance() {

    TRACE_PRINT(PGMSTR(message_ble_adding_char));
    TRACE_PRINTLN(PGMSTR(message_ble_ambiance));
 
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
      ERROR_PRINT(PGMSTR(message_ble_error_creating_char));
      ERROR_PRINTLN(PGMSTR(message_ble_ambiance));
    } 
}


void ChimeBluetooth::setup_attribute_uptime() {

    TRACE_PRINT(PGMSTR(message_ble_adding_char));
    TRACE_PRINTLN(PGMSTR(message_ble_uptime));
 
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
      ERROR_PRINT(PGMSTR(message_ble_error_creating_char));
      ERROR_PRINTLN(PGMSTR(message_ble_uptime));
    } 
}

void ChimeBluetooth::setup_char_actions() {

    TRACE_PRINT(PGMSTR(message_ble_adding_char));
    TRACE_PRINTLN(PGMSTR(message_ble_actions));
 
    // GATT_CHARS_PROPERTIES_WRITE
    bleCharActions = gatt.addCharacteristic(
      // format
      BLE_GATT_CHAR_ACTIONS,
      // properties
      GATT_CHARS_PROPERTIES_WRITE, 
      // data size min, max, 
      sizeof(ble_actions_bytes), sizeof(ble_actions_bytes), 
      // present format
      BLE_DATATYPE_BYTEARRAY
      );
    if (bleCharActions == 0) {
      ERROR_PRINT(PGMSTR(message_ble_error_creating_char));
      ERROR_PRINTLN(PGMSTR(message_ble_actions));
    } 
}

void ChimeBluetooth::decodeCurrentDateTime(uint8_t data[], uint16_t len) {

   // ensure the length is ok
   if (len != sizeof(ble_datetime_bytes)) {
      TRACE_PRINT(PGMSTR(message_wrong_size));
      TRACE_PRINTLN(len);
      return;
   } 
   
   // decode from bytes
   ble_datetime_bytes currentTime;
   memcpy(&currentTime, data, len);

  clock->receivedCurrentDateTime(currentTime.data);
  
}

void ChimeBluetooth::decodeAlarm1(uint8_t data[], uint16_t len) {

  // ensure the length is ok
  if (len != sizeof(ble_alarm_bytes)) {
    TRACE_PRINT(PGMSTR(message_wrong_size));
    TRACE_PRINTLN(len);
    return;
  } 

  // decode from bytes
  ble_alarm_bytes received_alarm;
  memcpy(&received_alarm, data, len);
  
  alarm1->receivedAlarm2(received_alarm.data);
}

void ChimeBluetooth::decodeAlarm2(uint8_t data[], uint16_t len) {

  // ensure the length is ok
  if (len != sizeof(ble_alarm_bytes)) {
    TRACE_PRINT(PGMSTR(message_wrong_size));
    TRACE_PRINTLN(len);
    return;
  } 

  // decode from bytes
  ble_alarm_bytes received_alarm;
  memcpy(&received_alarm, data, len);

  alarm2->receivedAlarm2(received_alarm.data);
}


void ChimeBluetooth::decodeAmbiance(uint8_t data[], uint16_t len) {
  
  // ensure the length is ok
  if (len != sizeof(ble_ambiance_bytes)) {
    TRACE_PRINT(PGMSTR(message_wrong_size));
    TRACE_PRINTLN(len);
    return;
  } 

  // decode from bytes
  ble_ambiance_bytes received_ambiance;
  memcpy(&received_ambiance, data, len);

  ambiance->receivedAmbiance(received_ambiance.data);

}

void ChimeBluetooth::decodeLightSettings(uint8_t data[], uint16_t len) {
  
  // ensure the length is ok
  if (len != sizeof(ble_light_settings_bytes)) {
    TRACE_PRINT(PGMSTR(message_wrong_size));
    TRACE_PRINTLN(len);
    return;
  } 

  // decode from bytes
  ble_light_settings_bytes received_settings;
  memcpy(&received_settings, data, len);

  lightSensor->receivedLightSettings(received_settings.data);

}

void ChimeBluetooth::decodeSoundSettings(uint8_t data[], uint16_t len) {
  
  // ensure the length is ok
  if (len != sizeof(ble_sound_settings_bytes)) {
    TRACE_PRINT(PGMSTR(message_wrong_size));
    TRACE_PRINTLN(len);
    return;
  } 

  // decode from bytes
  ble_sound_settings_bytes received_settings;
  memcpy(&received_settings, data, len);

  soundSensor->receivedSoundSettings(received_settings.data);
}

void ChimeBluetooth::decodeActions(uint8_t data[], uint16_t len) {
  
  // ensure the length is ok
  if (len != sizeof(ble_actions_bytes)) {
    TRACE_PRINT(PGMSTR(message_wrong_size));
    TRACE_PRINTLN(len);
    return;
  } 

  // decode from bytes
  ble_actions_bytes content;
  memcpy(&content, data, len);

  if (content.data.ring > 0) {
    switch (content.data.ring) {
      case 1: { chime->doDemoLight(); break; }
      case 2: { chime->doDemoMedium(); break; }
      case 3: { chime->doDemoStrong(); break; }
      default: {
        ERROR_PRINT(F("wrong action ring strength:")); ERROR_PRINTLN(content.data.ring);
      }
    }
  } 
  if (content.data.snooze) {
    // TODO
  }
  if (content.data.shutup) {
    // TODO
  }
  
}


void ChimeBluetooth::reactCharacteristicReceived(int32_t chars_id, uint8_t data[], uint16_t len) {

  DEBUG_PRINT(F("bluetooth: received val for char"));
  DEBUG_PRINTLN(chars_id);
  
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
  } else if (chars_id == bleCharActions) {
    decodeActions(data, len); 
  } else {
    ERROR_PRINT(F("ERROR: unknown bluetooth characteristic "));
    ERROR_PRINTLN(chars_id);
  }
}

/*void ChimeBluetooth::reactCharacteristicReceivedStatic(int32_t chars_id, uint8_t data[], uint16_t len) {
  ble_singleton->reactCharacteristicReceivedStatic(chars_id, data, len);
};*/

void ChimeBluetooth::reactCentralConnected() {
  TRACE_PRINT(PGMSTR(message_ble_bluetooth));
  TRACE_PRINT(F("central connected "));
 
  if (ble.sendCommandCheckOK(F("AT+BLEGETPEERADDR"))) {
    TRACE_PRINTLN(ble.buffer);
  } else {
    TRACE_PRINTLN('?');
  }
};


void ChimeBluetooth::reactCentralDisconnected()  {
  TRACE_PRINT(PGMSTR(message_ble_bluetooth));
  TRACE_PRINTLN(F("central disconnected"));
};

void ChimeBluetooth::publishAmbiance(ble_ambiance ambiance) {

  // TODO
  ble_ambiance_bytes content;
  content.data = ambiance;

  TRACE_PRINT(PGMSTR(message_ble_bluetooth_publishing));
  TRACE_PRINTLN(F("ambiance"));
  
  // update the corresponding attribute value
  gatt.setChar(bleCharAmbiance, content.bytes, sizeof(content.bytes));

}

void ChimeBluetooth::publishAlarm1(ble_alarm alarm) {

  // encore as bytes
  ble_alarm_bytes content;
  content.data = alarm;
  
  TRACE_PRINT(PGMSTR(message_ble_bluetooth_publishing));
  TRACE_PRINTLN(F("alarm1"));
  
  // update the corresponding attribute value
  gatt.setChar(bleCharAlarm1, content.bytes, sizeof(content.bytes));

}

void ChimeBluetooth::publishAlarm2(ble_alarm alarm) {

  // encore as bytes
  ble_alarm_bytes content;
  content.data = alarm;
  
  TRACE_PRINT(PGMSTR(message_ble_bluetooth_publishing));
  TRACE_PRINTLN(F("alarm2"));
  
  // update the corresponding attribute value
  gatt.setChar(bleCharAlarm2, content.bytes, sizeof(content.bytes));

}

void ChimeBluetooth::publishTemperature1(float temp) {

  // encore as bytes
  ble_float_bytes content;
  content.value = temp;
  
  TRACE_PRINT(PGMSTR(message_ble_bluetooth_publishing));
  TRACE_PRINTLN(F("temperature1"));
  
  // update the corresponding attribute value
  gatt.setChar(bleCharTemperature1, content.bytes, sizeof(content.bytes));

}

void ChimeBluetooth::publishTemperature2(float temp) {

  // encore as bytes
  ble_float_bytes content;
  content.value = temp;

  TRACE_PRINT(PGMSTR(message_ble_bluetooth_publishing));
  TRACE_PRINTLN(F("temperature2"));
  
  // update the corresponding attribute value
  gatt.setChar(bleCharTemperature2, content.bytes, sizeof(content.bytes));

}


void ChimeBluetooth::publishUptime(uint32_t uptime_min) {

  // encore as bytes
  ble_uint32_bytes content;
  content.value = uptime_min;

  TRACE_PRINT(PGMSTR(message_ble_bluetooth_publishing));
  TRACE_PRINTLN(F("uptime"));
  
  // update the corresponding attribute value
  gatt.setChar(bleCharUptime, content.bytes, sizeof(content.bytes));

}


void ChimeBluetooth::publishLightSensor(ble_light_sensor light_sensor) {

  // encore as bytes
  ble_light_sensor_bytes content;
  content.data = light_sensor;

  TRACE_PRINT(PGMSTR(message_ble_bluetooth_publishing));
  TRACE_PRINTLN(F("light sensor"));
  
  // update the corresponding attribute value
  gatt.setChar(bleCharLightSensor, content.bytes, sizeof(content.bytes));

}


void ChimeBluetooth::publishLightSettings(ble_light_settings settings) {

  // encore as bytes
  ble_light_settings_bytes content;
  content.data = settings;

  TRACE_PRINT(PGMSTR(message_ble_bluetooth_publishing));
  TRACE_PRINTLN(F("light settings"));
  
  // update the corresponding attribute value
  gatt.setChar(bleCharLightSettings, content.bytes, sizeof(content.bytes));

}

void ChimeBluetooth::publishSoundSensor(ble_sound_sensor sound_sensor) {

  // encore as bytes
  ble_sound_sensor_bytes content;
  content.data = sound_sensor;
  
  TRACE_PRINT(PGMSTR(message_ble_bluetooth_publishing));
  TRACE_PRINTLN(F("sound sensor"));
  
  // update the corresponding attribute value
  gatt.setChar(bleCharSoundSensor, content.bytes, sizeof(content.bytes));

}

void ChimeBluetooth::publishSoundSettings(ble_sound_settings settings) {

  // encore as bytes
  ble_sound_settings_bytes content;
  content.data = settings;

  TRACE_PRINT(PGMSTR(message_ble_bluetooth_publishing));
  TRACE_PRINTLN(F("sound settings"));
  
  // update the corresponding attribute value
  gatt.setChar(bleCharSoundSettings, content.bytes, sizeof(content.bytes));
  
}

/**
 * Reads whatever available from bluetooth
 */
void ChimeBluetooth::readAndReact() {

  // TODO only every few seconds!
  // is button pressed???
  if (digitalRead(pinButtonConnect) == LOW) {
    DEBUG_PRINTLN(F("button is pressed"));
  }
  if (digitalRead(pinButtonSwitch) == LOW) {
    DEBUG_PRINTLN(F("bluetooth is off"));
  }
  
  // check if anything new on the side of BLE?
  ble.update(200);

}

void ChimeBluetooth::setUsers(ChimeClock* _clock, ChimeAlarm* _alarm1, ChimeAlarm* _alarm2,
                  ChimeLightSensor* _lightSensor, ChimeSoundSensor* _soundSensor, 
                  Chime* _chime, Ambiance* _ambiance) {
  clock = _clock;
  alarm1 = _alarm1;
  alarm2 = _alarm2;
  lightSensor = _lightSensor;
  soundSensor = _soundSensor;
  chime = _chime;
  ambiance = _ambiance;
}

