
#define DEBUG

#ifdef DEBUG

  #define DEBUG_SETUP()       Serial.begin(9600)
  
  #define DEBUG_PRINT(x)      Serial.print(x)
  #define DEBUG_PRINT_DEC(x)  Serial.print(x,DEC)
  #define DEBUG_PRINTLN(x)    Serial.println(x)
 
#else

  #define DEBUG_SETUP()

  #define DEBUG_PRINT(x)
  #define DEBUG_PRINT_DEC(x)
  #define DEBUG_PRINTLN(x)
 
#endif





