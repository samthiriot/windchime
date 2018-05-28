
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

  delay(400);
  
  bluetoothConsume();
  readATResult(F("AT+RESET"));
  delay(500);
  
  //execAT(F("AT+RESET"));
  //delay(200);

  // define setup name
  /*
  String bluetoothName = readATResult(F("AT+NAME?"));   // get the current name of the module
  // TODO why does that execute ? 
  if (bluetoothName != F("CHIMUINO")) {                          // change the name if not initialized properly
    execAT(F("AT+NAMECHIMUINO"));                       // TODO if another chimuino already exists (scan !) then add a number ?
    DEBUG_PRINTLN(F("* set name to CHIMUINO"));
  } else {                            
    // TODO remove it
    DEBUG_PRINTLN(F("* name is already CHIMUINO"));
  }
  */
  
  // define services and characteristics
  // TODO define the right service!

  
  readATResult(F("AT+NOTI1"));
  readATResult(F("AT+ROLE0"));
  readATResult(F("AT+PASS050183"));
  readATResult(F("AT+NAMECHIMUINO"));
  readATResult(F("AT+RESET"));
  delay(500);

  readATResult(F("AT+NOTI1"));
  readATResult(F("AT+NAME?"));
  //readATResult(F("AT+NOTI0"));
  readATResult(F("AT+UUID0xFFE1"));
  readATResult(F("AT+CHAR0xFFE2"));
  
  //execAT(F("AT+ROLE0"));                                // define as a peripherical
  //execAT(F("AT+UUID0xFFE3"));                           // define a service
  //execAT(F("AT+CHAR0xFFE1"));                           // define a characteristic
  //execAT(F("AT+CHAR0xFFE2"));                           // define a characteristic
  //execAT(F("AT+NOTI1"));
  //execAT(F("AT+NOTI0"));

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
  bluetoothConsume();
  BTSerial.listen();

  BTSerial.println(cmd);
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
  char c;
  while (BTSerial.available()) {
    c = BTSerial.read();                                            // skip
    DEBUG_PRINT(F("bluetooth skip: ")); DEBUG_PRINTLN(c);
  }
}


/**
 * Executes an AT command, reads the 
 * result and returns it as a String
 */
String ChimeBluetooth::readATResult(String cmd) {

  DEBUG_PRINT(F("exec ")); DEBUG_PRINTLN(cmd);
  
  BTSerial.print(cmd);                                          // send to the command to the chip
  delay(1000);                                                   // gives time for the bluetooth chip to process the demand

  char serialdata[80];
  int count = 0;
  while (BTSerial.available() && count<80) 
  {
      serialdata[count++] = BTSerial.read();
  }
  serialdata[count] = '\0';

  DEBUG_PRINT(F("read AT result:")); DEBUG_PRINTLN(serialdata);

  // remove additional null
  String str = String(serialdata);
  //str.remove(str.length());

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

void ChimeBluetooth::sendDebug() {
  #ifdef DEBUG
  BTSerial << F("DEBUG") << endl;
  DEBUG_PRINTLN(F("bluetooth: sent debug"));
  #endif
}

void ChimeBluetooth::reactToCommand() {

  DEBUG_PRINT(F("bluetooth: processing command "));
  DEBUG_PRINTLN(received);
  char* received2 = received;
  // the AT messages might interfer; remove the known ones
  while (true) {
    if (
          (strncmp_P(received2, PSTR("OK+CONN"), 7) == 0) or 
          (strncmp_P(received2, PSTR("OK+LOST"), 7) == 0) ) {
      received2 = received2+7; 
      DEBUG_PRINT(F("bluetooth: processing instead command "));
      DEBUG_PRINTLN(received2);
    } else {
      break;
    }
  }
  
  // received
  // receivedCount
  char* received3 = received2;
  while (received3 < received + receivedCount - 2 ) {
    if (strncmp_P(received3, PSTR("GET "), 4) == 0) {
      // the command starts with GET
      processGet(received3 + 4);
      receivedCount = 0;
      return;
    } else if (strncmp_P(received3, PSTR("SET "), 4) == 0) {
      processSet(received3 + 4);
      receivedCount = 0;
      return;
    } else if (strncmp_P(received3, PSTR("DO "), 3) == 0) {
      processDo(received3 + 3);
      receivedCount = 0;
      return;
    }
    received3++;
  } 
  
  DEBUG_PRINT(F("ERROR: command ignored ")); DEBUG_PRINTLN(received2);
  receivedCount = 0;
  
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





