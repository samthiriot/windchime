
#include "ambiance.h"

#include <Streaming.h>

#include "debug.h"
#include "utils.h"

Ambiance::Ambiance(): BluetoothCommandListener() {

}

void Ambiance::setup() {
  
}

void Ambiance::debugSerial() {
  #ifdef DEBUG
  Serial << F("Ambiance: ") 
         << (enabled ? F("enabled") : F("disabled"))
         << endl;
  #endif 
}

BluetoothListenerAnswer Ambiance::processBluetoothGet(char* str, SoftwareSerial* BTSerial) {
  return NOT_CONCERNED;
}

BluetoothListenerAnswer Ambiance::processBluetoothSet(char* str, SoftwareSerial* BTSerial) {
  
  if (strncmp_P(str, PSTR("AMBIANCE "), 9) == 0) {

    DEBUG_PRINTLN(F("Ambiance changed by bluetooth"));

    char cEnabled;

    sscanf(str + 9,                                                  // decode received datetime
           "%c", 
           &cEnabled);    

    enabled = char2bool(cEnabled);

    debugSerial();

    return SUCCESS;
  }
  
  return NOT_CONCERNED;
}

BluetoothListenerAnswer Ambiance::processBluetoothDo(char* str, SoftwareSerial* BTSerial) {
  return NOT_CONCERNED;
}

