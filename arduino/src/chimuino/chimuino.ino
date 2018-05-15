// about bluetooth
// docs
// - https://medium.com/@yostane/using-the-at-09-ble-module-with-the-arduino-3bc7d5cb0ac2



// +---------------------------------------------+
// |      SKETCH SETTINGS                        |
// |                                             |
// +---------------------------------------------+
    
    // current version of the firmware.
    // should be upgraded
    #define FIRMWARE_VERSION "alpha_2018_05_10"
    
    #define DEBUG_SERIAL true                                         // config debug

// +---------------------------------------------+
// |      STEPPER                                |
// |          move the chime                     |
// +---------------------------------------------+
    
    // engine config 
    #define MOTOR_STEPS 2048                                          // in 4 steps mode
    
    #define MOTOR_SPEED_SLOW 5                                        // speed silencious
    #define MOTOR_SPEED_QUICK 10                                      // speed quick but less silencious
    #define MOTOR_SPEED_MAX 15                                        // max speed never to go beyond
    
    // how much to pull for every level ?
    #define MUSIC_PULL_VLIGHT MOTOR_STEPS/11
    #define MUSIC_PULL_LIGHT MOTOR_STEPS/10
    #define MUSIC_PULL_STRONG MOTOR_STEPS/9
    
    #define STEPPER_PIN_1 8
    #define STEPPER_PIN_2 9
    #define STEPPER_PIN_3 10
    #define STEPPER_PIN_4 11
    
    #include <Stepper.h>
    Stepper motor(MOTOR_STEPS, STEPPER_PIN_1, STEPPER_PIN_3, STEPPER_PIN_2, STEPPER_PIN_4);
    
    /**
     * Switch the stepper to free wheel: no noise, no energy consumption.
     */
    void motorFreeWheel() {
      digitalWrite(STEPPER_PIN_1, LOW);
      digitalWrite(STEPPER_PIN_2, LOW);
      digitalWrite(STEPPER_PIN_3, LOW);
      digitalWrite(STEPPER_PIN_4, LOW); 
    }
    
    void setupStepper() {
        pinMode(STEPPER_PIN_1, OUTPUT);
        pinMode(STEPPER_PIN_2, OUTPUT);
        pinMode(STEPPER_PIN_3, OUTPUT);
        pinMode(STEPPER_PIN_4, OUTPUT);
      
      #ifdef DEBUG_SERIAL
        Serial.println("init: going to position free");
      #endif
        motor.setSpeed(MOTOR_SPEED_SLOW);
        motor.step(MUSIC_PULL_VLIGHT);
        delay(100); 
        motor.step(-MUSIC_PULL_VLIGHT);
          
        motorFreeWheel();                                                     // relax and don't consume energy
    }
              
// +---------------------------------------------+
// |      CLOCK                                  |
// |          precise time and alarms            |
// +---------------------------------------------+
    #include <Wire.h>
    
    #include "RTClib.h"
    RTC_DS3231 rtc;

    bool alarmDisabled = false;                                           
    int alarmTimeHour = 7;                                                    // the hour 
    bool alarmFinished = false;                                               // true if we did the job of waking up the user

    void setupClock() {
      
      // init RTC !
      #ifdef DEBUG_SERIAL
        Serial.println("init: RTC...");
      #endif
        if (! rtc.begin()) {
      #ifdef DEBUG_SERIAL
          Serial.println("Couldn't find RTC");
      #endif
         while (1);
        }
      #ifdef DEBUG_SERIAL
        Serial.println("Found RTC :-)");
      #endif
    
      if (rtc.lostPower()) {
      #ifdef DEBUG_SERIAL
        Serial.println("RTC is NOT running! Initializing with compilation time.");
      #endif
         rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));                    //  sets the RTC to the date & time this sketch was compiled
      }
      #ifdef DEBUG_SERIAL
        Serial.println("init: RTC ok");
      #endif
    }

    void debugNow() {
      Serial.print("Time is: ");
      DateTime now = rtc.now();
      Serial.print(now.year(),    DEC); Serial.print('-');
      Serial.print(now.month(),   DEC); Serial.print('-');
      Serial.print(now.day(),     DEC); Serial.print(' ');
      Serial.print(now.hour(),    DEC); Serial.print(':');
      Serial.print(now.minute(),  DEC); Serial.print(':');
      Serial.println(now.second(), DEC);
    }
    
    void debugAlarm1() {
      Serial.print("Alarm:"); 
      if (alarmDisabled) 
        Serial.println("Disabled");
      else { 
        Serial.print(alarmTimeHour); Serial.println("h"); 
      }
      Serial.print("Alarm:"); Serial.println(alarmFinished?"Finished":"Active"); 
    }

// +---------------------------------------------+
// |      BLUETOOTH                              |
// |          for smartphone interaction         |
// +---------------------------------------------+
    
    // pins
    #define BLUETOOTH_RX 13
    #define BLUETOOTH_TX 12
    
    #include <SoftwareSerial.h>
    SoftwareSerial BTSerial(BLUETOOTH_RX, BLUETOOTH_TX);
    
    #define CR 13
    #define LR 10
    
    #define BLUETOOTH_LONGEST_COMMAND 50                             // the longest command we have to read (buffer size)
    
    /**
     * Executes an AT command
     * and reports result on the serial port
     */
    void bluetoothExecAT(String cmd) {
      BTSerial.print(cmd);
      BTSerial.listen();
      delay(800);
      while (BTSerial.available()) {
        // TODO change to a string accumulator
        Serial.write(BTSerial.read());
      }
    }
    
    /**
     * Reads and ignores any data
     * pending from bluetooth.
     */
    void bluetoothConsume() {
      BTSerial.listen();
      while (BTSerial.available()) {
        BTSerial.read();                                            // skip
      }
    }
    
    /**
     * Executes an AT command, reads the 
     * result and returns it as a String
     */
    String bluetoothReadATResult(String cmd) {
    
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
    
    
    /**
     * Reads a line from the bluetooth serial.
     * Returns null if nothing, else
     * a string if one was received.
     */
    String bluetoothReadline() {
      
      BTSerial.listen();                                            // listen to bluetooth
    
      // TODO update for a loop
      char concatenated[BLUETOOTH_LONGEST_COMMAND];
      int count = BTSerial.readBytesUntil(CR, concatenated, BLUETOOTH_LONGEST_COMMAND);
      //BTSerial.flush();
      // consume (burn) the available data
      while(BTSerial.available() > 0) {
        BTSerial.read();
      }
      
      String str = String(concatenated);
     
      return str; 
    }
    
    /**
     * Called at init only
     */
    void setupBluetooth() {
      
      pinMode(BLUETOOTH_RX, INPUT);                               // configure pins
      pinMode(BLUETOOTH_TX, OUTPUT);
      
      BTSerial.begin(9600);                                       // serial communication speed
    
      BTSerial.listen();                                          // by default, listen to bluetooth serial
        
      String bluetoothName = bluetoothReadATResult("AT+NAME?");   // get the current name of the module
      // TODO why does that execute ? 
      if (bluetoothName != "CHIMUINO") {                          // change the name if not initialized properly
        bluetoothExecAT("AT+NAMECHIMUINO");                       // TODO if another chimuino already exists (scan !) then add a number ?
      } else {                            
        // TODO remove it
        Serial.print("* name is already ");
        Serial.println(bluetoothName);
      }
    
      bluetoothExecAT("AT+ROLE0");                                // define as a peripherical
      bluetoothExecAT("AT+UUID0xFFE0");                           // define a service
      bluetoothExecAT("AT+CHAR0xFFE1");                           // define a characteristic
    
    }

// +---------------------------------------------+
// |      SOUND DETECTOR                         |
// |            sense the world                  |
// +---------------------------------------------+
    
    // config sound detector
    #define SOUND_PIN A1
    
    // what we know about sound
    
    int soundLevelMin = 0;                                                    // the lowest sound we ever measured
    int soundLevelMax = 1024;                                                 // the higest ever heard
    int soundBackgroundSilenceThreshold = 20;                                 // the silence / noise threshold
    int soundLastQuiet = 0;                                                   // the last time it was quiet 
    int soundLastNotQuiet = 0;                                                // the last time it was noisy
    
    void setupSoundSensor() {
        pinMode(SOUND_PIN, INPUT); 
        soundLastQuiet = soundLastNotQuiet  = 0;                              // let's say the last time it was quiet and noisy was far ago
        soundLevelMin = soundLevelMax = analogRead(SOUND_PIN);                // init the sound levels known so far
    }

// +---------------------------------------------+
// |      LIGHT SENSOR                           |
// |            simple but smart                 |
// +---------------------------------------------+
    
    #define PHOTOCELL_PIN A0                                                  // port for the photovoltaic cell
    #define PHOTOCELL_NIGHT_THRESHOLD 550                                     // TODO detect threshold
   
    bool wasDark = false;                                                     // true if was dark before

    void setupLightSensor() {
        pinMode(PHOTOCELL_PIN, INPUT);
    }

// +---------------------------------------------+
// |      RANDOM                                 |
// |            alea yacta est                   |
// +---------------------------------------------+
    
    #define RANDOM_PIN A2                                                     // refers to a pin unconnected supposed to catch white noise 
    
    void setupRandom() {
        pinMode(RANDOM_PIN, INPUT);                                           // this pin is used to init the random network generator
        int seed = analogRead(RANDOM_PIN);
      #ifdef DEBUG_SERIAL
        Serial.print("init: random seed is "); Serial.print(seed); Serial.println();
      #endif
        randomSeed(seed);
    }


enum mode {
  SILENCE,      // do not play sound
  BRUISSEMENT,  // barely hear it from time to time
  TINTEMENT,    // hear a very light bell sound
  PREREVEIL,    // hear the bell, enough to be conscious of their existence
  REVEIL,       // hear the bells so much it should awake you
  AMBIANCE      // put a good mood in the room; most of the time it's only bruissement, yet also tintement; from time to time it will to a prereveil so you're aware of it
};

mode current_mode = AMBIANCE;



/**
 * Entry point, ran before anything else by the Arduino.
 */
void setup() {
  
#ifdef DEBUG_SERIAL
  Serial.begin(9600); // 57600
  Serial.println("init: beginning !");
#endif

  setupLightSensor();
  setupSoundSensor();
  setupRandom();
  setupStepper();
  setupBluetooth();
  setupClock();

#ifdef DEBUG_SERIAL
  Serial.println("init: end.");
#endif

}

void bruissement() {

  #ifdef DEBUG_SERIAL
    Serial.println("bruissement...");
  #endif

  for (int i=0; i<random(1,3); i++){
    
    motor.setSpeed(MOTOR_SPEED_SLOW);
    motor.step(MUSIC_PULL_VLIGHT);
    delay(100); 
    motor.step(-MUSIC_PULL_VLIGHT);
    delay(1000);
    motorFreeWheel();

    // discoveredLevelForPosition(MUSIC_PULL_VLIGHT, readMaxSoundLevelAvgSec(5));
  }
  
}

void tintement() {

  #ifdef DEBUG_SERIAL
    Serial.println("tintement...");
  #endif

  for (int i=0; i<random(1,5); i++){
    
    motor.setSpeed(MOTOR_SPEED_SLOW);
    motor.step(MUSIC_PULL_LIGHT);
  
    delay(100);
    
    motor.setSpeed(MOTOR_SPEED_QUICK);
    motor.step(-MUSIC_PULL_LIGHT);

    delay(500);
  
    //discoveredLevelForPosition(MUSIC_PULL_LIGHT, readMaxSoundLevelAvgSec(5));
  }
    motorFreeWheel();

}

void prereveil() {

  #ifdef DEBUG_SERIAL
    Serial.println("prereveil...");
  #endif
  
  for (int i=0; i<4; i++){

    motor.setSpeed(MOTOR_SPEED_SLOW);
    motor.step(MUSIC_PULL_LIGHT);
  
    delay(100);
    
    motor.setSpeed(MOTOR_SPEED_QUICK);
    motor.step(-MUSIC_PULL_LIGHT);

    delay(300);
    
    //discoveredLevelForPosition(MUSIC_PULL_LIGHT, readMaxSoundLevelAvgSec(5));
  }
  motorFreeWheel();


}

void reveil() {

  #ifdef DEBUG_SERIAL
    Serial.println("reveil...");
  #endif
  
  for (int i=0; i<3; i++){
    motor.setSpeed(MOTOR_SPEED_QUICK);
    motor.step(MUSIC_PULL_STRONG);
  
    delay(100);
    
    motor.setSpeed(MOTOR_SPEED_QUICK);
    motor.step(-MUSIC_PULL_STRONG);

    delay(300);
    
  //discoveredLevelForPosition(POSITION_BELL_STRONG, readMaxSoundLevelAvgSec(3));
  }  
    motorFreeWheel();


}


/**
 * Reads the max of the value during the X (approximative) seconds.
 * Also update the general variables soundLevelMin and Max. 
 */
int readMaxSoundLevelAvgSec(int seconds) {

  int cumulated = 0;
  int count = 0;
  int sndMax = 0; 
  Serial.print("level: "); 
  for (int i=0; i<seconds*50; i++) {
      int sndLvl = analogRead(SOUND_PIN);
      // computation of max
      if (sndLvl > sndMax) {
        sndMax= sndLvl ;
      }
      // for the computation of mean
      cumulated += sndLvl;
      count++;
      if (sndLvl < soundLevelMin) {
        soundLevelMin = sndLvl ;
      } 
      // debug
      #ifdef DEBUG_SERIAL
      Serial.print(sndLvl); Serial.print(" ");
      if (i > 0 && i % 20 == 0) {
        Serial.println();
      }
      #endif
      delay(20);
  }
  int avg = cumulated / count;
  #ifdef DEBUG_SERIAL      
  Serial.print(" => max: "); Serial.print(sndMax); Serial.print(", mean: "); Serial.print(avg );
  Serial.println();
  #endif
  if (sndMax > soundLevelMax) {
    soundLevelMax= sndMax;
  }

    
  return avg ; // sndMax; //
  
}

int readSoundLevel() {
  int rawSoundLevel =  analogRead(SOUND_PIN);
  if (rawSoundLevel < soundLevelMin) {
    soundLevelMin = rawSoundLevel;
  } else if (rawSoundLevel > soundLevelMax) {
    soundLevelMax= rawSoundLevel;
  }
  return rawSoundLevel;
}

int measureSoundLevelForPosition(int pos) {
  
    #ifdef DEBUG_SERIAL
      Serial.print("calibrating sound sensor for pos "); Serial.println(pos);
    #endif
    //moveSlowTo(0);

    motor.setSpeed(MOTOR_SPEED_SLOW);
    
    for (int i=5; i>0; i--) {
      motor.step(pos);
      delay(50);
      motor.step(-pos);
      delay(200);
      //discoveredLevelForPosition(pos, readMaxSoundLevelAvgSec(i==0?10:5));
      //Serial.print(i); Serial.print("/ impact of pos "); Serial.print(pos); Serial.print(" => "); Serial.print(sndAfterMax); Serial.println();

    }
    //Serial.print("impact of pos "); Serial.print(pos); Serial.print(" => "); Serial.print(cumulated/5); Serial.println();
    //discoveredLevelForPosition(pos, cumulated/5);
    //return cumulated/10;

}

void calibrateServoWithSound() {
    
  
   #ifdef DEBUG_SERIAL
      Serial.println("calibrating sound sensor !");
    #endif
  int sndBefore = readMaxSoundLevelAvgSec(10);
    #ifdef DEBUG_SERIAL
      Serial.print("level before: "); Serial.print(sndBefore); Serial.println();
    #endif
    //discoveredLevelForPosition(0-5, sndBefore );

  // test all reasonnable positions by a step of 1
  for (int p=0; p<=MUSIC_PULL_STRONG ; p++) {
    measureSoundLevelForPosition(p);
    delay(1000);
 
  }
 

}


void bluetoothReactGetVersion() {
    BTSerial.print("VERSION IS ");
    BTSerial.println(FIRMWARE_VERSION);
}

void bluetoothReactGetDatetime() {
    BTSerial.print("DATETIME IS ");
    DateTime now = rtc.now();
    BTSerial.print(now.year(),    DEC); BTSerial.print('-');
    BTSerial.print(now.month(),   DEC); BTSerial.print('-');
    BTSerial.print(now.day(),     DEC); BTSerial.print(' ');
    BTSerial.print(now.hour(),    DEC); BTSerial.print(':');
    BTSerial.print(now.minute(),  DEC); BTSerial.print(':');
    BTSerial.println(now.second(), DEC);
}

void bluetoothReactGetAlarm1() {
    BTSerial.print("ALARM1 IS ");
    BTSerial.print(alarmTimeHour,     DEC); BTSerial.print(':');
    BTSerial.print(30,                DEC); BTSerial.print(' ');        // TODO !
    BTSerial.println(!alarmDisabled,  DEC); 
}

void bluetoothReactDoChime() {
    reveil();
    BTSerial.println("DOING CHIME");                                // acknowledge
}

void bluetoothReactSetDatetime(String str) {
    
    char cc[str.length()+1];                                        // convert to char*
    str.toCharArray(cc, str.length()+1);
    int year, month, day, hour, minutes, seconds;
    sscanf(cc,                                                      // decode received datetime
           "%u-%u-%u %u:%u:%u", 
           &year, &month, &day, &hour, &minutes, &seconds);    
           
    // TODO reject invalid time 
    
    rtc.adjust(DateTime(year, month, day, hour, minutes, seconds)); // store the novel datetime into the RTC clock
    
    BTSerial.write("DATETIME SET\n");                               // acknowledge
}
void bluetoothReactSetAlarm1(String str) {

    Serial.print("received alarm"); Serial.println(str);
    
    char cc[str.length()+1];                                        // convert to char*
    str.toCharArray(cc, str.length()+1);
    int hour, minutes, enabled;                                     // decode alarm
    sscanf(cc, "%u:%u %u", &hour, &minutes, &enabled);                
    
    alarmTimeHour = hour;                                           // store novel state
    // TODO minutes !
    minutes = 30;
    alarmDisabled = (enabled == 0);
    
    Serial.print("Alarm:"); 
    if (alarmDisabled) Serial.println("Disabled");
    else { Serial.print(alarmTimeHour); Serial.println("h"); }
}

/**
 * Listens to bluetooth, and reacts to messages 
 * when a complete message is available.
 */
void bluetoothListenAndReact() {

  Serial.println("reading bluetooth...");                           
  String received = bluetoothReadline();                            // read data from bluetooth (if any)
  if (!received.length()) { return; }                               // no message; return
    
  Serial.print("received from bluetooth: ");
  Serial.println(received);
  
  if (received == "GET VERSION")          { bluetoothReactGetVersion();     }
  else if (received == "GET DATETIME")    { bluetoothReactGetDatetime();    }
  else if (received == "GET ALARM1")      { bluetoothReactGetAlarm1();      }

  else if (received == "DO CHIME")        { bluetoothReactDoChime();    }
  

  else if (received.startsWith("SET DATETIME "))  { bluetoothReactSetDatetime(received.substring(14)); }
  else if (received.startsWith("SET ALARM1 "))    { bluetoothReactSetAlarm1(received.substring(11)); }
    
}


void loop() {

  //for (int i=0; i<50; i++) {
    bluetoothListenAndReact();
  //  delay(500);
  //}
  
  current_mode = NULL;

  // EXTERNAL CONTEXT SENSORS: how is the world around ? 

  // is there any light around? 
  bool isDark;
  {
    int photocellTension = analogRead(PHOTOCELL_PIN);
    #ifdef DEBUG_SERIAL
      Serial.print("light: "); Serial.print(photocellTension); 
    #endif
    // ex:
    // 1000 lumière soleil indirect lumière
    // 980 with full lamp 
    // 810 without this lamp
    // 600 half covered
    // 290 full covered
    isDark = (photocellTension < PHOTOCELL_NIGHT_THRESHOLD);
    Serial.print(" => ");
    Serial.print(isDark ? "night" : "daylight");
    Serial.println();
  }


  // what time is it ? 
  DateTime now = rtc.now();
  debugNow();
  
  // is there sound around? which frequencies? 
  int sndAvg = readMaxSoundLevelAvgSec(1);
  bool isQuietNow = sndAvg  <= soundBackgroundSilenceThreshold ;
  bool isQuiet;
  if (isQuietNow){
    soundLastQuiet = now.unixtime();
    isQuiet = soundLastQuiet - soundLastNotQuiet > 31; // it is quiet is there was no sound since et least 60 seconds
     Serial.print("now "); Serial.print(soundLastQuiet); Serial.print(" last quiet "); Serial.print(soundLastQuiet); Serial.print(" diff"); Serial.print(soundLastQuiet - soundLastNotQuiet);
  } else{
    soundLastNotQuiet = now.unixtime();
    isQuiet = false;
  }

  
  #ifdef DEBUG_SERIAL
     Serial.print("sound: "); Serial.print(sndAvg ); Serial.print(" [");Serial.print(soundLevelMin); Serial.print("-"); Serial.print(soundLevelMax); Serial.print("] => ");
     Serial.print("it is "); Serial.println(isQuiet ? "quiet..." : "not quiet.");
     
  #endif


  // EXTERNAL USER INPUTS: what does he want? 
  

  // should we put some ambiance? 

  // should we ring the hours? 

  // was I knocked? 

  // INTERNAL SENSORS: what do we know ? 

  // what time is it? 

  // do we have an alarm clock?
  debugAlarm1();
  if (!alarmDisabled and !alarmFinished) {
    
    // we have something to do later ! 
    if (now.hour() == alarmTimeHour) {
      // that the time guys !
      if (now.minute() >= 15) {
        if (now.minute() <= 30){
          Serial.println("The guy should wake up soon! Let's pass into PREREVEIL."); 
          current_mode = PREREVEIL;     
        } else {
          current_mode = REVEIL;
          Serial.println("The guy should wake up now ! Let's pass into REVEIL."); 
        }
      } 
    }
  }

  // UNDERSTAND THE WORLD: what effect had our past action?

  
  // DESIRE ACTION: what should we do?

  // is it time for alarm clock now or soon? 

  // is there a hour to ring? (if its not night...)

  // should we put a good mood? (if its not already noisy around !)

  if (current_mode == NULL){
    
    if (isDark) {
      current_mode = SILENCE;
      #ifdef DEBUG_SERIAL
        Serial.println("It is night; will not play ambiance."); 
      #endif
  
      if (not wasDark) {
        // wow, it's just night. let's relax our servo to be sure it will not be destroyed overnight
        //  TODO ???
        // moveSlowTo(POSITION_MIN);
        // delay(50);
      }
    } else if (wasDark) {
      // but but... it was dark and it's light now ! let's say welcome to the world !
      #ifdef DEBUG_SERIAL
        Serial.println("Night is over! Hello sunlight!"); 
      #endif
      // moveSlowTo(0);
      //delay(50);
      current_mode = REVEIL;
      /*
      for (int i=0; i<10; i++){
        strongest(); // TODO remove !
      }
      */
    } else if (isQuiet) {
      current_mode = AMBIANCE;
    }
  }
  
  // MEMORIZE THE WORLD
  wasDark = isDark;
  
  // ACT !
   // do some music
  switch (current_mode) {
    case SILENCE:
      delay(1000);
      break;
    case BRUISSEMENT:
      bruissement();
      break;
    case TINTEMENT:
      tintement();
      break;  
    case PREREVEIL:
      prereveil();
      delay(random(5000,60000));
      break;
    case REVEIL:
      reveil();
      delay(random(30*1000l,5*60*1000l));
      break;
    case AMBIANCE:

      // TODO another mode
      int r = random(0,100);
      if (r <= 60) { 
        // nothing
      } else if (r <= 65) {
        reveil();
        delay(random(4*60*1000l,15*60*1000l));
      } else if (r <= 75) {
        prereveil();
        delay(random(4*60*1000l,10*60*1000l));      
      } else if (r <= 80) {
        tintement();
        delay(random(2*60*1000l,8*60*1000l));

      } else {
        bruissement();
        delay(random(1*10*1000l,7*60*1000l));

      }
      break;
  }  
 
  
}
