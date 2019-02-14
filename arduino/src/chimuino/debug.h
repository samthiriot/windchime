
// keep commented if you want debug, error or trace
#define SERIAL

// see errors
#define ERROR
// see debug messavges
#define DEBUG
// see a lot of details
// #define TRACE

#include <Arduino.h>

const char msg_ok_dot[] PROGMEM  = { "ok."};

// use to access strings
#define PGMSTR(x) (__FlashStringHelper*)(x)


#ifdef SERIAL

  #define DEBUG_SETUP()       Serial.begin(115200)
  
#else

  #define DEBUG_SETUP()

#endif


#ifdef DEBUG
  
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



#ifdef TRACE

  #define TRACE_PRINT(x)      Serial.print(x)
  #define TRACE_PRINT_DEC(x)  Serial.print(x,DEC)
  #define TRACE_PRINTLN(x)    Serial.println(x)

#else

  #define TRACE_PRINT(x) 
  #define TRACE_PRINT_DEC(x)
  #define TRACE_PRINTLN(x) 

#endif 


#ifdef ERROR

  #define ERROR_PRINT(x)      Serial.print(x)
  #define ERROR_PRINTLN(x)    Serial.println(x)

#else

  #define ERROR_PRINT(x) 
  #define ERROR_PRINTLN(x)

#endif 




