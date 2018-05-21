
#include "chime_bluetooth.h"

#include "debug.h"

// special characters
#define CR 13
#define LR 10

#include <Arduino.h>

ChimeBluetooth::ChimeBluetooth(unsigned short _pinRX, unsigned short _pinTX):
      BTSerial(_pinRX, _pinTX) {

  pinRX = _pinRX;
  pinTX = _pinTX;
  
  // BTSerial = new SoftwareSerial(pinRX, pinTX);

}

void ChimeBluetooth::setup() {

  DEBUG_PRINTLN(F("init: bluetooth..."));

  // configure pins
  pinMode(pinRX, INPUT);                               
  pinMode(pinTX, OUTPUT);
    
  BTSerial.begin(9600);                                       // serial communication speed
  
  BTSerial.listen();                                          // by default, listen to bluetooth serial

  bluetoothConsume();

  execAT(F("AT+RESET"));
  delay(200);
  
  // define setup name
  String bluetoothName = readATResult(F("AT+NAME?"));   // get the current name of the module
  // TODO why does that execute ? 
  if (bluetoothName != F("CHIMUINO")) {                          // change the name if not initialized properly
    execAT(F("AT+NAMECHIMUINO"));                       // TODO if another chimuino already exists (scan !) then add a number ?
  } else {                            
    // TODO remove it
    Serial.print(F("* name is already "));
    Serial.println(bluetoothName);
  }

  // define services and characteristics
  // TODO define the right service!
  execAT(F("AT+ROLE0"));                                // define as a peripherical
  execAT(F("AT+UUID0xFFE0"));                           // define a service
  execAT(F("AT+CHAR0xFFE1"));                           // define a characteristic

  DEBUG_PRINTLN(F("init: bluetooth ok"));
}

void ChimeBluetooth::addCommandInterpreter(BluetoothCommandListener* listener) {
  if (listeners_count >= BLUETOOTH_MAX_LISTENERS) {
    // cannot add another listener, all the slots are taken :-(
    // TODO warn
    return;
  }
  listeners[listeners_count++] = listener;
}

/**
 * Executes an AT command
 * and reports result on the serial port
 */
void ChimeBluetooth::execAT(String cmd) {
  BTSerial.print(cmd);
  BTSerial.listen();
  delay(800);                                       // wait a bit so the command is processed

  char c;
  while (BTSerial.available()) {
    c = (byte)BTSerial.read();
    DEBUG_PRINT(c);
  }
}

 
/**
 * Reads and ignores any data
 * pending from bluetooth.
 */
void ChimeBluetooth::bluetoothConsume() {
  BTSerial.listen();
  while (BTSerial.available()) {
    BTSerial.read();                                            // skip
  }
}


/**
 * Executes an AT command, reads the 
 * result and returns it as a String
 */
String ChimeBluetooth::readATResult(String cmd) {

  bluetoothConsume();                                           // consume older things
  BTSerial.print(cmd);                                          // send to the command to the chip
  BTSerial.listen();
  delay(200);                                                   // gives time for the bluetooth chip to process the demand

  char serialdata[80];
  int count = 0;

  // skip before : 
  char r = BTSerial.available() ? BTSerial.read() : 0;
  while (BTSerial.available() && (r !=':') ) {
    // skip
    r = BTSerial.read();
  }
    
  // read data until EOL
  count = BTSerial.readBytesUntil(CR, serialdata, 80);

  // remove additional null
  String str = String(serialdata);
  str.remove(str.length());

  DEBUG_PRINT(F("read AT result:")); DEBUG_PRINTLN(str);
  
  return str;
}

void ChimeBluetooth::processGet(char* str) {
  DEBUG_PRINT(F("bluetooth: received GET "));
  DEBUG_PRINTLN(str);
  // call each listener and submit this possibility
  for (int i=0; i<listeners_count; i++) {
    BluetoothListenerAnswer answer = listeners[i]->processBluetoothGet(str, &BTSerial);  
    switch(answer) {
      case SUCCESS:
        // finished :-)
        return;
      case FAILURE:
        DEBUG_PRINT(F("ERROR: failed processing bluetooth GET ")); DEBUG_PRINTLN(str);
        break;
      case NOT_CONCERNED:
        break;  
    }
  }
  // no one used our message
  DEBUG_PRINT(F("WARN: no one used bluetooth GET "));
  DEBUG_PRINTLN(str);
}

void ChimeBluetooth::processSet(char* str) {
  DEBUG_PRINT(F("bluetooth: received SET "));
  DEBUG_PRINTLN(str);
  // call each listener and submit this possibility
  for (int i=0; i<listeners_count; i++) {
    BluetoothListenerAnswer answer = listeners[i]->processBluetoothSet(str, &BTSerial);  
    switch(answer) {
      case SUCCESS:
        return;
      case FAILURE:
        DEBUG_PRINT(F("ERROR: failed processing bluetooth SET ")); DEBUG_PRINTLN(str);
        break;
      case NOT_CONCERNED:
        break;  
    }
  }
  // no one used our message
  DEBUG_PRINT(F("WARN: no one used bluetooth command SET "));
  DEBUG_PRINTLN(str);
}

void ChimeBluetooth::processDo(char* str) {
  DEBUG_PRINT(F("bluetooth: received DO "));
  DEBUG_PRINTLN(str);
  // call each listener and submit this possibility
  for (int i=0; i<listeners_count; i++) {
    BluetoothListenerAnswer answer = listeners[i]->processBluetoothDo(str, &BTSerial);  
    switch(answer) {
      case SUCCESS:
        return;
      case FAILURE:
        DEBUG_PRINT(F("ERROR: failed processing bluetooth DO ")); DEBUG_PRINTLN(str);
        break;
      case NOT_CONCERNED:
        break;  
    }
  }
  // no one used our message
  DEBUG_PRINT(F("WARN: no one used bluetooth command DO "));
  DEBUG_PRINTLN(str);
}

void ChimeBluetooth::reactToCommand() {
  DEBUG_PRINT(F("bluetooth: processing command "));
  DEBUG_PRINTLN(received);
  // received
  // receivedCount
  if (strncmp_P(received, PSTR("GET "), 4) == 0) {
    // the command starts with GET
    processGet(received + 4);
  } else if (strncmp_P(received, PSTR("SET "), 4) == 0) {
    processSet(received + 4);
  } else if (strncmp_P(received, PSTR("DO "), 3) == 0) {
    processDo(received + 3);
  } else {
    DEBUG_PRINT(F("ERROR: command ignored "));
    DEBUG_PRINTLN(received);
    bluetoothConsume();
    receivedCount = 0;
  }
}

/**
 * Reads whatever available from bluetooth
 */
void ChimeBluetooth::readAndReact() {

  //BTSerial.listen();
  
  char c;
  while(BTSerial.available()) {                             // there is something to read
    c = (byte)BTSerial.read();                              // read it
    //DEBUG_PRINT(F("bluetooth: received "));
    //DEBUG_PRINTLN(c);
    if (c == '\n') {                                        // detect the end of a command
      if (receivedCount > 0) {
        //DEBUG_PRINTLN(F("bluetooth: end of command"));
        received[ receivedCount ] = '\0';                     // terminate the accumulated string
        //bluetoothCommandAvailable = true;                     // the command is available for processing
        reactToCommand();                            // use the command
        receivedCount = 0;                           // restart reading from scratch
      }
    } else if (c == '\0') {
      // ignore some chars
    } else if (receivedCount < BLUETOOTH_LONGEST_COMMAND-1) {
      //DEBUG_PRINT(F("bluetooth: added to command")); DEBUG_PRINTLN(receivedCount);
      received[ receivedCount++ ] = c;    // accumulate the character 
    } else {                                                // OOPS, seems like we have overflowed our buffer capabilities
      DEBUG_PRINTLN(F("ERROR: OVERFLOW OF BLUETOOTH BUFFER !?"));     // warn 
      DEBUG_PRINTLN(received);
      receivedCount = 0;                           // reset the reading (will probably not lead to something that good)
    }
  }
  
}





