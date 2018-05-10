#include <Servo.h>

// config debug
#define DEBUG_SERIAL true

// config photocell
#define PHOTOCELL_PIN A5

#define PHOTOCELL_NIGHT_THRESHOLD 750

// config connection servo
#define SERVO_1_PIN A0
#define SERVO_DELAY_COMPLETE 800

#define MOVE_SLOW_DELAY_STEP 50
#define MOVE_SLOW_INC_STEP 1

#define MOVE_QUICK_DELAY_STEP 10
#define MOVE_QUICK_INC_STEP 2

// config physique
#define POSITION_MIN 10
#define POSITION_MAX 180
#define POSITION_FREE 70
#define POSITION_BELL_SOFT 95
#define POSITION_BELL_NORMAL 105
#define POSITION_BELL_STRONG 125
#define POSITION_BELL_STRONGER 135

#define POSITION_AROUND 5


enum mode {
  SILENCE,      // do not play sound
  BRUISSEMENT,  // barely hear it from time to time
  TINTEMENT,    // hear a very light bell sound
  PREREVEIL,    // hear the bell, enough to be conscious of their existence
  REVEIL,       // hear the bells so much it should awake you
  AMBIANCE      // put a good mood in the room; most of the time it's only bruissement, yet also tintement; from time to time it will to a prereveil so you're aware of it
};

mode current_mode = AMBIANCE;


// our servo
Servo servo1;
int pos = 0;    // variable to store the servo position 

// previous dark state
bool wasDark = false;

void moveImmediateTo(int p)
{
    pos = p;
    servo1.write(p);
}

void moveSlowTo(int p) 
{
  if (p > pos) {  
    for (; pos <= p; pos=pos+MOVE_SLOW_INC_STEP) {
      servo1.write(pos);
      delay(MOVE_SLOW_DELAY_STEP);
    }
  } else {
    for (; pos >= p; pos=pos-MOVE_SLOW_INC_STEP) {
      servo1.write(pos);
      delay(MOVE_SLOW_DELAY_STEP);
    }
  }
}

void moveQuickTo(int p) 
{
  
  if (p > pos) {
    for (; pos <= p; pos=pos+MOVE_QUICK_INC_STEP) {
      servo1.write(pos);
      delay(MOVE_QUICK_DELAY_STEP);
    }  
  } else {
    for (; pos >= p; pos=pos-MOVE_QUICK_INC_STEP) {
      servo1.write(pos);
      delay(MOVE_QUICK_DELAY_STEP);
    }  
  }
  
}

void setup()
{
#ifdef DEBUG_SERIAL
  Serial.begin(9600);
  Serial.println("init: beginning !");
#endif

  // config pins
  // ... photocell pins
  pinMode(PHOTOCELL_PIN, INPUT);
  // ... servo pins
  pinMode(SERVO_1_PIN, OUTPUT);
  servo1.attach(SERVO_1_PIN);

  // random
  int seed = analogRead(4);
  randomSeed(seed);

#ifdef DEBUG_SERIAL
  Serial.print("init: random seed is "); Serial.print(seed); Serial.println();
#endif
 
#ifdef DEBUG_SERIAL
  Serial.println("init: going to position min");
#endif
 
moveImmediateTo(POSITION_MIN);

#ifdef DEBUG_SERIAL
  Serial.println("init: at position min");
#endif

delay(50);

#ifdef DEBUG_SERIAL
  Serial.println("init: going to position free");
#endif

moveSlowTo(POSITION_FREE);

#ifdef DEBUG_SERIAL
  Serial.println("init: at position free");
#endif

delay(1000);


#ifdef DEBUG_SERIAL
  Serial.println("init: end.");
#endif

}

void bruissement() {

  #ifdef DEBUG_SERIAL
    Serial.println("bruissement...");
  #endif
  
  moveSlowTo(random(POSITION_BELL_SOFT, POSITION_BELL_SOFT+POSITION_AROUND));
  delay(10);
  moveSlowTo(POSITION_FREE);
    
  // bruissement: 
  delay(random(8000,60000));

}

void tintement() {

  #ifdef DEBUG_SERIAL
    Serial.println("tintement...");
  #endif
  
  moveSlowTo(random(POSITION_BELL_SOFT, POSITION_BELL_SOFT+POSITION_AROUND));
  delay(10);
  moveSlowTo(POSITION_FREE);
    
  // tintement: 
  delay(random(5000,60000));

}

void prereveil() {

  #ifdef DEBUG_SERIAL
    Serial.println("prereveil...");
  #endif
  
  moveSlowTo(POSITION_BELL_NORMAL);
  delay(10);
  moveQuickTo(POSITION_FREE);
    
  // tintement: 
  delay(random(5000,20000));

}

void reveil() {

  #ifdef DEBUG_SERIAL
    Serial.println("reveil...");
  #endif
  
  moveSlowTo(POSITION_BELL_STRONG);
  delay(10);
  moveQuickTo(POSITION_FREE);
    
  // tintement: 
  delay(random(5000,10000));

}

void strongest() {

  #ifdef DEBUG_SERIAL
    Serial.println("reveil...");
  #endif
  
  moveQuickTo(POSITION_BELL_STRONGER);
  delay(100);
  moveQuickTo(POSITION_FREE);
    
  // tintement: 
  delay(random(1000,5000));

}

void loop() {

  // EXTERNAL CONTEXT SENSORS: how is the world around ? 

  // is there any light around? 
  bool isDark;
  {
    int photocellTension = analogRead(PHOTOCELL_PIN);
    #ifdef DEBUG_SERIAL
      Serial.print("light: "); Serial.print(photocellTension); Serial.println();
    #endif
    // ex:
    // 1000 lumière soleil indirect lumière
    // 980 with full lamp 
    // 810 without this lamp
    // 600 half covered
    // 290 full covered
    isDark = (photocellTension < PHOTOCELL_NIGHT_THRESHOLD);
  }
    
  // is there sound around? which frequencies? 

  
  // EXTERNAL USER INPUTS: what does he want? 
  
  // what is the programmed alarm clock, if any?

  // should we put some ambiance? 

  // should we ring the hours? 

  // was I knocked? 

  // INTERNAL SENSORS: what do we know ? 

  // what time is it? 

  // do we have an alarm clock?


  // UNDERSTAND THE WORLD: what effect had our past action?

  
  // DESIRE ACTION: what should we do?

  // is it time for alarm clock now or soon? 

  // is there a hour to ring? (if its not night...)

  // should we put a good mood? (if its not already noisy around !)
  if (isDark) {
    current_mode = SILENCE;
    #ifdef DEBUG_SERIAL
      Serial.println("It is night; will not put ambiance."); 
    #endif

    if (not wasDark) {
      // wow, it's just night. let's relax our servo to be sure it will not be destroyed overnight
      moveSlowTo(POSITION_MIN);
      delay(50);
    }
  } else if (wasDark) {
    // but but... it was dark and it's light now ! let's say welcome to the world !
    #ifdef DEBUG_SERIAL
      Serial.println("Night is over! Hello sunlight!"); 
    #endif
    moveSlowTo(POSITION_FREE  );
    delay(50);
    current_mode = REVEIL;
    /*
    for (int i=0; i<10; i++){
      strongest(); // TODO remove !
    }
    */
  } else {
    current_mode = AMBIANCE;
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
      break;
    case REVEIL:
      reveil();
      break;
    case AMBIANCE:
      int r = random(0,100);
      if (r <= 5) {
        reveil();
      } else if (r <= 40) {
        prereveil();
      } else if (r <= 90) {
        tintement();
      } else {
        bruissement();
      }
      break;
  }  
  
}
