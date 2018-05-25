
// +---------------------------------------------+
// |      BLUETOOTH                              |
// |          for smartphone interaction         |
// +---------------------------------------------+
 
#ifndef CHIME_BLUETOOTH_H
#define CHIME_BLUETOOTH_H

#include <SoftwareSerial.h>


#define BLUETOOTH_LONGEST_COMMAND 200                             // the longest command we have to read (buffer size)
#define BLUETOOTH_MAX_LISTENERS 10

enum BluetoothListenerAnswer {
  NOT_CONCERNED,
  SUCCESS,
  FAILURE
};

class BluetoothCommandListener {
    public:
 
      // called when a GET <SOMETHING> was received
      // will receive as arguments the <SOMETHING> char*, 
      // and a pointer to serial port to use to answer
      virtual BluetoothListenerAnswer processBluetoothGet(char* str, SoftwareSerial* BTSerial);

      // called when a SET <SOMETHING> <OTHER THINGS> was received
      // will receive as arguments the <SOMETHING> <OTHER THINGS> char*
      // and a pointer to serial port to use to answer
      virtual BluetoothListenerAnswer processBluetoothSet(char* str, SoftwareSerial* BTSerial);

      // called when a DO <SOMETHING> was received
      // will receive as arguments the <SOMETHING> char*
      // and a pointer to serial port to use to answer
      virtual BluetoothListenerAnswer processBluetoothDo(char* str, SoftwareSerial* BTSerial);
};

class ChimeBluetooth {
  
   private:

    BluetoothCommandListener *listeners[BLUETOOTH_MAX_LISTENERS] ;
    unsigned short listeners_count = 0;
    
    SoftwareSerial BTSerial;
   
    // hardware config
    unsigned short pinRX, pinTX;

    // state 
    uint8_t receivedCount = 0;
    char received[BLUETOOTH_LONGEST_COMMAND];      // the current accumulation of the string read from bluetooth

    // consumes (and looses) any available info from the chip
    void bluetoothConsume();
    // react to a command accumulated in the received variable
    void reactToCommand();
    void processGet(char* str);
    void processSet(char* str);
    void processDo(char* str);
    
  public:
    ChimeBluetooth(unsigned short pinRX, unsigned short pinTX);
    void setup();

    void sendDebug();
    
    // reads commands from bluetooth, and reacts if relevant
    void readAndReact();
    // sends an AT command to the chip
    void execAT(String cmd);
    // TODO change to char* ?
    String readATResult(String cmd);
    // add something able to deal with commands
    void addCommandInterpreter(BluetoothCommandListener* listener);

};


#endif // CHIME_BLUETOOTH_H






