
//#define DEBUG

#include <Arduino.h>

#ifdef DEBUG


  #define DEBUG_SETUP()       Serial.begin(9600)
  
  #define DEBUG_PRINT(x)      Serial.print(x)
  #define DEBUG_PRINT_DEC(x)  Serial.print(x,DEC)
  #define DEBUG_PRINTLN(x)    Serial.println(x)

  #include <avr/pgmspace.h>

  const char message_init [] PROGMEM = "init: ";

#else

  #define DEBUG_SETUP()

  #define DEBUG_PRINT(x)
  #define DEBUG_PRINT_DEC(x)
  #define DEBUG_PRINTLN(x)
 
#endif





