// about bluetooth
// docs
// - https://medium.com/@yostane/using-the-at-09-ble-module-with-the-arduino-3bc7d5cb0ac2


// to control the stepper engine 
#include <Stepper.h>

// for RTC time 
#include <Wire.h>
#include "RTClib.h"

// for bluetooth
#include <SoftwareSerial.h>

#define FIRMWARE_VERSION "alpha_2018_05_10"

// config debug
#define DEBUG_SERIAL true

// config photocell
#define PHOTOCELL_PIN A0
#define PHOTOCELL_NIGHT_THRESHOLD 550

// config sound detector
#define SOUND_PIN A1

// config touch (test)
#define TOUCH_H_PIN A2

// config random init
#define RANDOM_PIN A2

// config bluetooth
#define BLUETOOTH_RX 13
#define BLUETOOTH_TX 12
SoftwareSerial BTSerial(BLUETOOTH_RX, BLUETOOTH_TX);
#define CR 13
#define LR 10

// config connection servo

// engine config 
// .. number of steps
#define MOTOR_STEPS 2048 // in 4 steps mode

// ... speed silencious
#define MOTOR_SPEED_SLOW 5
// ... speed quick but less silencious
#define MOTOR_SPEED_QUICK 10
// .. max speed never to go beyond
#define MOTOR_SPEED_MAX 15

// how much to pull for every level ?
#define MUSIC_PULL_VLIGHT MOTOR_STEPS/11
#define MUSIC_PULL_LIGHT MOTOR_STEPS/10
#define MUSIC_PULL_STRONG MOTOR_STEPS/9


Stepper motor(MOTOR_STEPS, 8, 10, 9, 11);

 
#define MOVE_SLOW_DELAY_STEP 20
#define MOVE_SLOW_INC_STEP 1

#define MOVE_QUICK_DELAY_STEP 10
#define MOVE_QUICK_INC_STEP 2

enum mode {
  SILENCE,      // do not play sound
  BRUISSEMENT,  // barely hear it from time to time
  TINTEMENT,    // hear a very light bell sound
  PREREVEIL,    // hear the bell, enough to be conscious of their existence
  REVEIL,       // hear the bells so much it should awake you
  AMBIANCE      // put a good mood in the room; most of the time it's only bruissement, yet also tintement; from time to time it will to a prereveil so you're aware of it
};

mode current_mode = AMBIANCE;

// our RTC module
RTC_DS3231 rtc;

// previous dark state
bool wasDark = false;

// what we know about sound
// ... the lowest we ever measured
int soundLevelMin = 0;  
// ... and the higher
int soundLevelMax = 1024;
// ... the background sound level
int soundBackgroundSilenceThreshold = 20;
// ... last times if was quiet and noisy
int soundLastQuiet = 0;
int soundLastNotQuiet = 0;


// alarm 
bool alarmDisabled = false;
int alarmTimeHour = 7;
// ... true if we did the job of waking up the user
bool alarmFinished = false;

#define LEARNING_POSITIONS_COUNT  // first is lower than, last is higher than upper and lower values

/*
#define LEARNING_POSITIONS_COUNT MUSIC_PULL_STRONG-0+2 // first is lower than, last is higher than upper and lower values
int position2level[LEARNING_POSITIONS_COUNT];

int getIndexForPosition(int pos) {
  int pos5 = pos/5;
  if (pos5 < LEARNING_POSITIONS_MIN) {
    return 0;
  }
  if (pos5 > LEARNING_POSITIONS_MAX+1) {
    return LEARNING_POSITIONS_COUNT-1;
  }
  return pos5-LEARNING_POSITIONS_MIN;
}
void discoveredLevelForPosition(int pos, int lvl) {
  // TODO
 // just replace the value
 if (position2level[getIndexForPosition(pos)] < 0) {
  position2level[getIndexForPosition(pos)] = lvl;
 } else {
  position2level[getIndexForPosition(pos)] = (position2level[getIndexForPosition(pos)]*3+lvl)/4;
 }

 printPositions();
}

void printPositions() {
  for (int i=0; i<LEARNING_POSITIONS_COUNT; i++) {
    // for position...
    if (i == 0) {
      //Serial.print("<"); Serial.print(LEARNING_POSITIONS_MIN*5+5);     
    } else if (i == LEARNING_POSITIONS_COUNT-1) {
      //Serial.print(">"); Serial.print(LEARNING_POSITIONS_MIN*5+(i-1)*5); 
    } else {
     // Serial.print(LEARNING_POSITIONS_MIN*5+i*5); 
    } 
    // we know level...
    Serial.print(": level="); 
    if (position2level[i] < 0) {
      Serial.print("?"); 
    } else {
      Serial.print(position2level[i]); 
    }
    
    Serial.println();
  }
  
}
*/


void bluetoothExecAT(String cmd) {

  //BTSerial.write("AT+");
   // delay(20);


  BTSerial.print(cmd);
    
  delay(200);
  BTSerial.listen();
  delay(800);
  
  while (BTSerial.available()) {
    // TODO change to a string accumulator
    Serial.write(BTSerial.read());
    //delay(10);
  }
  //delay(1000);
}


/**
 * Reads and ignores any data
 * pending from bluetooth.
 */
void bluetoothConsume() {
  BTSerial.listen();
  while (BTSerial.available()) {
    // skip
    BTSerial.read();
  }
}


/**
 * Executes an AT command, reads the 
 * result and returns it as a String
 */
String bluetoothReadATResult(String cmd) {

  // consume older things
  bluetoothConsume();
  
  BTSerial.print(cmd);

  // gives time for the bluetooth chip
  // to process the demand 
  delay(200);

  BTSerial.listen();

  char serialdata[80];
  int count = 0;
  
  // read data until EOL

  // skip before : 
  char r = BTSerial.available() ? BTSerial.read() : 0;
  while (BTSerial.available() && (r !=':') ) {
    // skip
    r = BTSerial.read();
  }
  
  count = BTSerial.readBytesUntil(CR, serialdata, 80);

  // remove additional null
  String str = String(serialdata);
  str.remove(str.length()-1);
  
  Serial.print("read:");
  Serial.println(str);
  
  return str;
}

#define BLUETOOTH_LONGEST_COMMAND 50

/**
 * returns null if nothing, else
 * a string if one was received.
 */
String bluetoothReadline() {
  
  BTSerial.listen();

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

void setupBluetooth() {
  
  // configure pins
  pinMode(BLUETOOTH_RX, INPUT);
  pinMode(BLUETOOTH_TX, OUTPUT);
  
  // define the speed 
  // TODO quicker ? 
  BTSerial.begin(9600);  

  // TODO if another chimuino already exists (scan !)
  // then add a number ?
  // set the name of the module, if not correct already
  String bluetoothName = bluetoothReadATResult("AT+NAME?");
  // why does that execute ? 
  if (!bluetoothName.equals(String("CHIMUINO"))) {
    //Serial.println("* defining bluetooth name");
    // we have to change the name 
    bluetoothExecAT("AT+NAMECHIMUINO");
    //delay(1000);
  } else {
    Serial.print("* name is already ");
    Serial.println(bluetoothName);
  }

  bluetoothExecAT("AT+ROLE0");
  bluetoothExecAT("AT+UUID0xFFE0");
  bluetoothExecAT("AT+CHAR0xFFE1");

}

void setup()
{

#ifdef DEBUG_SERIAL
  Serial.begin(9600); // 57600
  Serial.println("init: beginning !");
#endif

  // config pins
  // ... photocell pins
  pinMode(PHOTOCELL_PIN, INPUT);
  // ... sound detector
  pinMode(SOUND_PIN, INPUT);
  // ... morot
  pinMode(8, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(11, OUTPUT);
  
  // .. random pin
  pinMode(RANDOM_PIN, INPUT);
  // ... touch
  pinMode(TOUCH_H_PIN, INPUT);
  digitalWrite(TOUCH_H_PIN, INPUT_PULLUP);
  // TODO pullup ?

  setupBluetooth();

  // init random 
  int seed = analogRead(RANDOM_PIN);
  #ifdef DEBUG_SERIAL
  Serial.print("init: random seed is "); Serial.print(seed); Serial.println();
  #endif
  randomSeed(seed);


  // init what we know
  // ... we don't know the level of sound for various positions
  // TODO for (int i=0; i<LEARNING_POSITIONS_COUNT; i++) { position2level[i] = -1; }

  
  // ... let's say the last time it was quiet and noisy was far ago
  soundLastQuiet = soundLastNotQuiet  = 0;
  
// TODO debug
//readMaxSoundLevelAvgSec(60*5);

// init engine
  
  #ifdef DEBUG_SERIAL
    Serial.println("init: going to position free");
  #endif

  motor.setSpeed(MOTOR_SPEED_SLOW);
  motor.step(MUSIC_PULL_VLIGHT);
  delay(100); 
  motor.step(-MUSIC_PULL_VLIGHT);
  
  #ifdef DEBUG_SERIAL
    Serial.println("init: at position free");
  #endif
  motorFreeWheel();

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
    Serial.println("Found RTC !");
    #endif


  if (rtc.lostPower()) {
    #ifdef DEBUG_SERIAL
    Serial.println("RTC is NOT running! Initializing with compilation time.");
    #endif
    
    // following line sets the RTC to the date & time this sketch was compiled
     rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
     // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
  #ifdef DEBUG_SERIAL
  Serial.println("init: RTC ok");
  #endif

 // init sound
 soundLevelMin = soundLevelMax = analogRead(SOUND_PIN);

//delay(1000);

//calibrateServoWithSound();

//printPositions();


#ifdef DEBUG_SERIAL
  Serial.println("init: end.");
#endif

}

void motorFreeWheel() {
  digitalWrite(8, LOW);
  digitalWrite(9, LOW);
  digitalWrite(10, LOW);
  digitalWrite(11, LOW);
  
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

void detectTouchHour() {

  int touched = analogRead(TOUCH_H_PIN);
  Serial.print("touched: "); Serial.print(touched); Serial.println();

  bool alarmDisabledNew = (touched > 1018);
  int alarmHour = -1;
  if (alarmDisabledNew){
    if (!alarmDisabled) {
      alarmDisabled = alarmDisabledNew;
      Serial.print("Alarm is now disabled"); Serial.println();
      alarmFinished = false;
    }
    return;
  }
  
  // read which hour
  if (touched > 1000) {
    alarmHour = 1;
  } else if (touched > 870) {
    alarmHour = 4;
  } else if (touched > 830) {
    alarmHour = 5;
  } else if (touched > 780) {
    alarmHour = 6;
  } else if (touched > 715) {
    alarmHour = 7;
  } else if (touched > 685) {
    alarmHour = 8;
  } else if (touched > 520) {
    alarmHour = 11;
  } else if (touched > 450) {
    alarmHour = 14;
  } else {
    alarmHour = 23;
  }
  if (alarmTimeHour != alarmHour) {
    alarmTimeHour = alarmHour;
    alarmFinished = false;
    alarmDisabled = false;
  
  }
  Serial.print("Alarm is set to "); Serial.print(alarmHour); Serial.print("h 30"); Serial.println();

}

void bluetoothListenAndReact() {

  // read data from bluetooth (if any)
  Serial.println("reading bluetooth...");
  String received = bluetoothReadline();
  if (!received.length()) {
    // no message; return
    return;
  }
  Serial.print("received from bluetooth: ");
  Serial.println(received);
  if (received == "GET VERSION") {
    BTSerial.write("VERSION IS ");
    BTSerial.write(FIRMWARE_VERSION);
    BTSerial.write("\n");
  } else if (received == "DO CHIME") {
    reveil();
    BTSerial.write("DOING CHIME\n");
  } else if (received.startsWith("SET DATETIME ")) {
    Serial.println("received datetime");
    // decode time 
    uint32_t timestamp;
    String str = received.substring(14);
    char cc[str.length()];
    str.toCharArray(cc, str.length());
    int year, month, day;
    int hour, minutes, seconds;
    sscanf(cc, "%u-%u-%u %u:%u:%u", &year, &month, &day, &hour, &minutes, &seconds);
    // TODO reject invalid time 
    // store it into the RTC clock
    rtc.adjust(DateTime(year, month, day, hour, minutes, seconds));
    DateTime now = rtc.now();
  Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(' ');
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
  Serial.println();
    BTSerial.write("DATETIME SET\n");
  }
  
}
void loop() {

  for (int i=0; i<50; i++) {
    bluetoothListenAndReact();
    delay(500);
  }
  
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
  Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(' ');
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
  Serial.println();
  
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
  
  // what is the programmed alarm clock, if any?
  // TODO detectTouchHour();

  // should we put some ambiance? 

  // should we ring the hours? 

  // was I knocked? 

  // INTERNAL SENSORS: what do we know ? 

  // what time is it? 

  // do we have an alarm clock?
  Serial.print("Alarm:"); 
  if (alarmDisabled) Serial.println("Disabled");
  else { Serial.print(alarmTimeHour); Serial.println("h"); }
  //Serial.println(alarmDisabled ?"Disabled":"Enabled"); 
  Serial.print("Alarm:"); Serial.println(alarmFinished?"Finished":"Active"); 
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
  
  //current_mode = REVEIL;

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
