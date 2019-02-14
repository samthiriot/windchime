
// +---------------------------------------------+
// |      BLUETOOTH                              |
// |          for smartphone interaction         |
// +---------------------------------------------+

/**
 * The bluetooth class manages the exchanges with the bluetooth dongle:
 * publish information that central devices read upon demand, and 
 * receive information from smartphone applications which adapt user settings
 * or ask for actions. 
 * 
 * The bluetooth class knows the consumers of information (reference), and 
 * when an information is received, they call the corresponding function of
 * the consumer to notify him of the new information.
 * 
 * The information producers do know the bluetooth class. When they are initialized,
 * or later when information changes, they call the corresponding function 
 * of the bluetooth class to publish this updated information.
 * 
 */
 
// TODO manage the connectable status !
// TODO use the AT+DFUIRQ to use IRQ so we wake up only when data is available
// TODO detect when the temperature is too high and alert (?)

// see https://github.com/adafruit/Adafruit_BluefruitLE_nRF51/blob/master/changelog_firmware.md

#ifndef CHIME_BLUETOOTH_H
#define CHIME_BLUETOOTH_H

#include <SoftwareSerial.h>

// includes for Bluetooth Low Energy
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_UART.h"
#include "Adafruit_BLEGatt.h"
//#include "IEEE11073float.h"

const uint8_t BLUEFRUIT_UART_CTS_PIN=9;
const uint8_t BLUEFRUIT_UART_MODE_PIN=8;
const uint8_t BLUEFRUIT_SWUART_TXD_PIN=11;
const uint8_t BLUEFRUIT_SWUART_RXD_PIN=10;
const uint8_t BLUEFRUIT_UART_RTS_PIN=12;

// the services and characteristics we will publish
#define BLE_GATT_SERVICE_SENSING      0x181A // https://www.bluetooth.com/specifications/assigned-numbers/environmental-sensing-service-characteristics
#define BLE_GATT_CHAR_TEMPERATURE     0x2A6E // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.characteristic.temperature.xml
#define BLE_GATT_CHAR_TEMPERATURE1    0x5561 // temperature from RTC clock
#define BLE_GATT_CHAR_TEMPERATURE2    0x5562 // temperature from BLE dongle
#define BLE_GATT_CHAR_LIGHT_SENSOR    0x5563
#define BLE_GATT_CHAR_LIGHT_SETTINGS  0x5564
#define BLE_GATT_CHAR_SOUND_SENSOR    0x5565
#define BLE_GATT_CHAR_SOUND_SETTINGS  0x5566

#define BLE_GATT_SERVICE_CHIMUINO     0x5550
#define BLE_GATT_CHAR_TIME            0x2A08 // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.characteristic.date_time.xml
#define BLE_GATT_CHAR_ALARM1          0x5551
#define BLE_GATT_CHAR_ALARM2          0x5552
#define BLE_GATT_CHAR_AMBIANCE        0x5553 // settings such as "enable chiming"
#define BLE_GATT_CHAR_UPTIME          0x5554 // in minutes
#define BLE_GATT_CHAR_ACTIONS         0x5555 // in minutes



// if true, the initialization of the dongle is verbose
#define BLE_VERBOSE_MODE            true

// if defined, we will do a factory reset
#define BLE_FACTORYRESET_ENABLE

// stores a date-time content, in order to update the current date time from a phone
struct ble_datetime {
  uint16_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t minutes;
  uint8_t seconds;
};
// stores an alarm content, in order to send it and retrieve it from a phone
struct ble_alarm {
  bool active;
  uint8_t hour;
  uint8_t minutes;
  bool sunday;
  bool monday;
  bool tuesday;
  bool wednesday;
  bool thursday;
  bool friday;
  bool saterday;
  uint8_t duration_soft;
  uint8_t duration_strong;
};

// stores the ambiance content
struct ble_ambiance {
  bool enabled;
};

struct ble_light_sensor {
  uint8_t level;
  uint8_t min;
  uint8_t max;
  bool isDark;
};

struct ble_light_settings {
  uint8_t threshold;
};


struct ble_sound_sensor {
  uint8_t level;
  uint8_t min;
  uint8_t max;
  bool isQuiet;
};

struct ble_sound_settings {
  uint8_t threshold;
};

struct ble_actions {
  uint8_t ring;     // 0 no ring; 1 ring slow; 3 ring strong 
  bool snooze;      // if true, snooze (else 0)
  uint16_t shutup;  // be quiet for that many minutes (or 0)
};
  
enum BluetoothListenerAnswer {
  NOT_CONCERNED,  // returned by a listener who does not care about the info
  PROCESSED       // returned by a listener who consumed the information. Often it means we can stop calling listeners.
};

class BluetoothUser;
class ChimeClock;
class ChimeAlarm;
class ChimeLightSensor; 
class ChimeSoundSensor;
class Chime; 
class Ambiance;


class ChimeBluetooth {

  private:

    ChimeClock* clock;
    ChimeAlarm* alarm1;
    ChimeAlarm* alarm2;
    ChimeLightSensor* lightSensor; 
    ChimeSoundSensor* soundSensor;
    Chime* chime;
    Ambiance* ambiance;
                  
    // the internal identifiers for the services and characteristics
    int32_t bleServiceChimuinoId;
    int32_t bleCharCurrentTime;
    int32_t bleCharAlarm1;
    int32_t bleCharAlarm2;
    int32_t bleCharAmbiance;
    int32_t bleCharUptime;
    int32_t bleCharActions;

    int32_t bleServiceSensingId;
    int32_t bleCharTemperature;
    int32_t bleCharTemperature1;
    int32_t bleCharTemperature2;
    int32_t bleCharLightSensor;
    int32_t bleCharLightSettings;
    int32_t bleCharSoundSensor;
    int32_t bleCharSoundSettings;

    SoftwareSerial bluefruitSS = SoftwareSerial(BLUEFRUIT_SWUART_TXD_PIN, BLUEFRUIT_SWUART_RXD_PIN);    
    Adafruit_BluefruitLE_UART ble = Adafruit_BluefruitLE_UART(bluefruitSS, BLUEFRUIT_UART_MODE_PIN, BLUEFRUIT_UART_CTS_PIN, BLUEFRUIT_UART_RTS_PIN);
    Adafruit_BLEGatt gatt = Adafruit_BLEGatt(ble);

    // declarations of services and of the corresponding characteristics
    void setup_service_chimuino();
    void setup_char_temperature();
    void setup_char_light_sensor();
    void setup_char_light_settings();
    void setup_char_sound_sensor();
    void setup_char_sound_settings();
    void setup_char_actions();

    void setup_service_sensing();
    void setup_attribute_current_time();
    void setup_attribute_alarm1();
    void setup_attribute_alarm2();
    void setup_attribute_ambiance();
    void setup_attribute_temperature1();
    void setup_attribute_temperature2();
    void setup_attribute_uptime();

    // for each data characteristic we might receive, we implement here 
    // the conversion from a bytes array to the relevant content
    void decodeCurrentDateTime(uint8_t data[], uint16_t len);
    void decodeAlarm1(uint8_t data[], uint16_t len);
    void decodeAlarm2(uint8_t data[], uint16_t len);
    void decodeAmbiance(uint8_t data[], uint16_t len);
    void decodeLightSettings(uint8_t data[], uint16_t len);
    void decodeSoundSettings(uint8_t data[], uint16_t len);
    void decodeActions(uint8_t data[], uint16_t len);

  public:
    ChimeBluetooth(unsigned short _pinTXD, unsigned short _pinRXD,
                   unsigned short _pinMode, unsigned short _pinCTS, unsigned short _pinRTS);
    void setup();

    // called when a central device connected to BLE
    void reactCentralConnected();
    void reactCentralDisconnected();
    void reactCharacteristicReceived(int32_t chars_id, uint8_t data[], uint16_t len);

    void sendDebug();

    // reads commands from bluetooth, and reacts if relevant
    void readAndReact();

    void setUsers(ChimeClock* _clock, ChimeAlarm* _alarm1, ChimeAlarm* _alarm2,
                  ChimeLightSensor* _lightSensor, ChimeSoundSensor* _soundSensor, 
                  Chime* _chime, Ambiance* _ambiance);
  
    // for each data characteristic we publish, we offer here
    // to change the data which is broadcasted
    void publishAmbiance(ble_ambiance content);
    void publishAlarm1(ble_alarm content);
    void publishAlarm2(ble_alarm content);
    void publishTemperature1(float temp);
    void publishTemperature2(float temp);
    void publishUptime(uint32_t uptime_mins);
    void publishLightSensor(ble_light_sensor content);
    void publishLightSettings(ble_light_settings content);
    void publishSoundSensor(ble_sound_sensor content);
    void publishSoundSettings(ble_sound_settings content);


};


/**
 * Components which produce information for bluetooth 
 * know the bluetooth component, and have a function to  
 * update the information published by bluetooth.
 **/
class BluetoothInformationProducer {
  protected:
    ChimeBluetooth* bluetooth;

    /**
       Publish our state (if any) to blutooth so
       bluetooth shares the right information.
    */
    virtual void publishBluetoothData() {};

  public:

    /**
       Called to inform a bluetooth user that
       the corresponding component is available and ready.
       It's a good time to publish data in bluetooth
       and register to listen bluetooth.
    */
    void setBluetooth(ChimeBluetooth* _bluetooth) {
      // store the object
      bluetooth = _bluetooth;
      // also broadcast information
      this->publishBluetoothData();
    };

 
};

#endif // CHIME_BLUETOOTH_H






