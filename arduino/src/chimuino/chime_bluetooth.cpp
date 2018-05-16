
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

  // configure pins
  pinMode(pinRX, INPUT);                               
  pinMode(pinTX, OUTPUT);
    
  BTSerial.begin(9600);                                       // serial communication speed
  
  BTSerial.listen();                                          // by default, listen to bluetooth serial

  // define setup name
  String bluetoothName = readATResult("AT+NAME?");   // get the current name of the module
  // TODO why does that execute ? 
  if (bluetoothName != "CHIMUINO") {                          // change the name if not initialized properly
    execAT("AT+NAMECHIMUINO");                       // TODO if another chimuino already exists (scan !) then add a number ?
  } else {                            
    // TODO remove it
    Serial.print("* name is already ");
    Serial.println(bluetoothName);
  }

  // define services and characteristics
  execAT("AT+ROLE0");                                // define as a peripherical
  execAT("AT+UUID0xFFE0");                           // define a service
  execAT("AT+CHAR0xFFE1");                           // define a characteristic
  
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
  str.remove(str.length()-1);
  
  Serial.print("read:"); Serial.println(str);                   // debug on serial
  
  return str;
}

void ChimeBluetooth::processGet(char* str) {
  // call each listener and submit this possibility
  for (int i=0; i<listeners_count; i++) {
    BluetoothListenerAnswer answer = listeners[i]->processBluetoothGet(str, &BTSerial);  
    switch(answer) {
      case SUCCESS:
        // finished :-)
        return;
      case FAILURE:
        DEBUG_PRINT("ERROR: failed processing bluetooth GET");
        DEBUG_PRINTLN(str);
        break;
      case NOT_CONCERNED:
        break;  
    }
  }
  // no one used our message
  DEBUG_PRINT("WARN: failed processing bluetooth GET");
  DEBUG_PRINTLN(str);
}

void ChimeBluetooth::processSet(char* str) {
  // call each listener and submit this possibility
  for (int i=0; i<listeners_count; i++) {
    BluetoothListenerAnswer answer = listeners[i]->processBluetoothSet(str, &BTSerial);  
    switch(answer) {
      case SUCCESS:
        return;
      case FAILURE:
        DEBUG_PRINT("ERROR: failed processing bluetooth GET");
        DEBUG_PRINTLN(str);
        break;
      case NOT_CONCERNED:
        break;  
    }
  }
  // no one used our message
  DEBUG_PRINT("WARN: failed processing bluetooth GET");
  DEBUG_PRINTLN(str);
}

void ChimeBluetooth::processDo(char* str) {
  // call each listener and submit this possibility
  for (int i=0; i<listeners_count; i++) {
    BluetoothListenerAnswer answer = listeners[i]->processBluetoothDo(str, &BTSerial);  
    switch(answer) {
      case SUCCESS:
        return;
      case FAILURE:
        DEBUG_PRINT("ERROR: failed processing bluetooth GET");
        DEBUG_PRINTLN(str);
        break;
      case NOT_CONCERNED:
        break;  
    }
  }
  // no one used our message
  DEBUG_PRINT("WARN: failed processing bluetooth GET");
  DEBUG_PRINTLN(str);
}

void ChimeBluetooth::reactToCommand() {
  // received
  // receivedCount
  if (strncmp(received, "GET ", 4) == 0) {
    // the command starts with GET
    processGet(received + 4);
  } else if (strncmp(received, "SET ", 4) == 0) {
    processSet(received + 4);
  } else if (strncmp(received, "DO ", 3) == 0) {
    processDo(received + 3);
  } else {
    DEBUG_PRINT("ERROR: command ignored ");
    DEBUG_PRINTLN(received);
  }
}

/**
 * Reads whatever available from bluetooth
 */
void ChimeBluetooth::readAndReact() {

  BTSerial.listen();
  
  char c;
  while(BTSerial.available()) {                             // there is something to read
    c = (byte)BTSerial.read();                              // read it
    if (c == '\n') {                                        // detect the end of a command
      received[ receivedCount ] = '\0';   // terminate the accumulated string
      //bluetoothCommandAvailable = true;                     // the command is available for processing
      reactToCommand();                            // use the command
      receivedCount = 0;                           // restart reading from scratch
    } else if (receivedCount < BLUETOOTH_LONGEST_COMMAND-1) {
      received[ receivedCount++ ] = c;    // accumulate the character 
    } else {                                                // OOPS, seems like we have overflowed our buffer capabilities
      #ifdef DEBUG_SERIAL
      Serial.println("OVERFLOW OF BLUETOOTH BUFFER !");     // warn 
      #endif
      receivedCount = 0;                           // reset the reading (will probably not lead to something that good)
    }
  }
  
}





