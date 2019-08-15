#include <Arduino.h>
#include <U8g2lib.h>
#include <stdlib.h>

#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

#include <Snooze.h>


// snooze: Load drivers
SnoozeDigital digital;

Snoozelc5vBuffer  lc5vBuffer;

SnoozeBlock config_teensyLC(digital, lc5vBuffer);



// Create an IntervalTimer object 
IntervalTimer myTimer;

const int ledPin = LED_BUILTIN;  // the pin with a LED


// The interrupt will blink the LED, and keep
// track of how many times it has blinked.
int ledState = LOW;
volatile bool onePPS = false;

// functions called by IntervalTimer should be short, run as quickly as
// possible, and should avoid calling other functions if possible.
void onePPSTick(void) {
  onePPS = true;
}


/* Constructor */
U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);


#define HOUR_BUTTON    15
#define MIN_BUTTON     14



char buf[20];
byte hour = 0;
byte minute = 0;
byte second = 5;
bool timedOut = false;
byte toggle = 0;
int alarmCnt = 0;

bool updateDisplay = true;
int tick = 0;


const unsigned long debounceDelay = 20;

int sleeping = false;

void setup(void)
{

  pinMode(ledPin, OUTPUT);
  pinMode(HOUR_BUTTON, INPUT_PULLUP);
//  attachInterrupt(HOUR_BUTTON, hour_butt_ISR, RISING);
  pinMode(MIN_BUTTON, INPUT_PULLUP);
//  attachInterrupt(MIN_BUTTON, min_butt_ISR, RISING);


    /********************************************************
     Define digital pins for waking the teensy up. This
     combines pinMode and attachInterrupt in one function.
     
     Teensy LC
     Digtal pins: 2,6,7,9,10,11,16,21,22
     ********************************************************/
//    digital.pinMode(15, INPUT_PULLUP, RISING);//pin, mode, type
    digital.pinMode(22, INPUT_PULLUP, RISING);//pin, mode, type

#if 0
    /********************************************************
     Set Low Power Timer wake up in milliseconds.
     ********************************************************/
    timer.setTimer(10000);// milliseconds

#endif

  u8g2.begin();
  u8g2.setFlipMode(0);


  myTimer.begin(onePPSTick, 1000000);  // to run every second
}


volatile bool  hour_button_flag = false;
volatile bool  min_button_flag = false;


void loop(void)
{

  static unsigned long force_sleep = 0;
  
  // debounce buttons
  hour_button_flag = hour_button_debounce();
  min_button_flag = min_button_debounce();

#if 0
  if (hour_button_flag && (digitalRead(HOUR_BUTTON) == LOW)) {
    hour_button_flag = false;
    force_sleep = millis();
  }


  if (hour_button_flag && (digitalRead(HOUR_BUTTON) == HIGH)) {
    hour_button_flag = false;
    updateDisplay = true;

    if ((millis() - force_sleep) > 3000) {
      timedOut = true;
    }
    if (timedOut) {
      alarmCnt = 1;
//      timedOut = false;
//      second = 5;
    } else {
      hour++;
      if (hour > 23)
        hour = 0;
    }
  }
  
#else
  if (hour_button_flag) {
    hour_button_flag = false;
    if (digitalRead(HOUR_BUTTON) == HIGH) {
      updateDisplay = true;

      if ((millis() - force_sleep) > 3000) {
        timedOut = true;
      }
      if (timedOut) {
        alarmCnt = 1;
      } else {
        hour++;
        if (hour > 23)
          hour = 0;
      }
    } else {
      force_sleep = millis();
    }
  }
#endif

  if (min_button_flag && (digitalRead(MIN_BUTTON) == HIGH)) {
    min_button_flag = false;
    updateDisplay = true;
    if (timedOut) {
      alarmCnt = 1;
//      timedOut = false;
//      second = 5;
    } else {
      if (sleeping) {
        sleeping = false;
      } else {
        minute += 5;
        if (minute > 59)
          minute -= 60;
      }
    }
  }

  if (updateDisplay) {
    updateDisplay = false;
    displayUpdate();
  }
  

  if (onePPS) {
    onePPS = false;
    toggle++;
    updateDisplay = true;
 
    if (!timedOut) {
      if (second) {
        second--;
      } else if (minute) {
        minute--;
        second = 59;
      } else if (hour) {
        hour--;
        minute = 59;
        second = 59;
      } else {
        u8g2.clearDisplay();
        timedOut = true;
        alarmCnt = 60 * 60;;
//  myTimer.begin(onePPSTick, 500000);  // blinkLED to run every second
            // alarm
      }
    } else {
      alarmCnt--;
      if (alarmCnt == 0) {
        myTimer.end();
        alarmCnt = 0;

        // turn on pinChange interrupt so we can get going again when a button is pushed
        //attachInterrupt(HOUR_BUTTON, hour_butt_ISR, RISING);
        attachInterrupt(MIN_BUTTON, min_butt_ISR, RISING);
        u8g2.setPowerSave(1);

        // go to sleep to conserve power
        sleeping = true;
        Snooze.hibernate( config_teensyLC );// return module that woke processor
    
      }
    }
  }
}


void min_butt_ISR(void)
{
    second = 5;
    minute = 0;
    hour = 0;
    timedOut = false;
    myTimer.begin(onePPSTick, 1000000);  // blinkLED to run every second
//detachInterrupt(interrupt)
//detachInterrupt(digitalPinToInterrupt(pin));
    detachInterrupt(MIN_BUTTON);
    u8g2.setPowerSave(0);
}

#if 0
void hour_butt_ISR(void)
{
  hour_button_flag = true;
}
#endif


void displayUpdate(void)
{
  u8g2.firstPage();
  
  do {

    u8g2.setFontMode(1);  // Transparent
    u8g2.setFontDirection(0);
    u8g2.setFont(u8g2_font_inb24_mf);

    if (second&1) {
      sprintf(buf, "%2d:%02d", hour, minute);
    } else {
      sprintf(buf, "%2d %02d", hour, minute);
    }
//    u8g2.drawFrame(0,0,128,64);
      
    if (!timedOut) {
      u8g2.drawStr(10,40,buf);
    } else {
      drawHappyFace(0,0);
      drawHappyFace(64,0);
    }
  } while ( u8g2.nextPage() );
}



#if 1
void drawHappyFace(int x, int y)
{
  if (toggle&1) {
    if (x==0) {
      u8g2.drawFilledEllipse(20+x, 20+y, 5, 8, U8G2_DRAW_ALL);
      u8g2.drawEllipse(40+x, 20+y, 5, 8, U8G2_DRAW_ALL);
    } else {
      u8g2.drawEllipse(20+x, 20+y, 5, 8, U8G2_DRAW_ALL);
      u8g2.drawFilledEllipse(40+x, 20+y, 5, 8, U8G2_DRAW_ALL);
    }
  } else {
    if (x==0) {
      u8g2.drawEllipse(20+x, 20+y, 5, 8, U8G2_DRAW_ALL);
      u8g2.drawFilledEllipse(40+x, 20+y, 5, 8, U8G2_DRAW_ALL);
    } else {
      u8g2.drawFilledEllipse(20+x, 20+y, 5, 8, U8G2_DRAW_ALL);
      u8g2.drawEllipse(40+x, 20+y, 5, 8, U8G2_DRAW_ALL);
    }
  }
  u8g2.drawCircle(30+x, 30+y, 20, U8G2_DRAW_LOWER_LEFT);
  u8g2.drawCircle(30+x, 30+y, 20, U8G2_DRAW_LOWER_RIGHT);
  u8g2.drawCircle(30+x, 31+y, 20, U8G2_DRAW_LOWER_LEFT);
  u8g2.drawCircle(30+x, 31+y, 20, U8G2_DRAW_LOWER_RIGHT);
  u8g2.drawCircle(30+x, 32+y, 20, U8G2_DRAW_LOWER_LEFT);
  u8g2.drawCircle(30+x, 32+y, 20, U8G2_DRAW_LOWER_RIGHT);
}
#else
void drawHappyFace(int x, int y)
{
  if (toggle&1) {
    if (x==0) {
      u8g2.drawDisc(20+x, 20+y, 5, U8G2_DRAW_ALL);
      u8g2.drawCircle(40+x, 20+y, 5, U8G2_DRAW_ALL);
    } else {
      u8g2.drawCircle(20+x, 20+y, 5, U8G2_DRAW_ALL);
      u8g2.drawDisc(40+x, 20+y, 5, U8G2_DRAW_ALL);
    }
  } else {
    if (x==0) {
      u8g2.drawCircle(20+x, 20+y, 5, U8G2_DRAW_ALL);
      u8g2.drawDisc(40+x, 20+y, 5, U8G2_DRAW_ALL);
    } else {
      u8g2.drawDisc(20+x, 20+y, 5, U8G2_DRAW_ALL);
      u8g2.drawCircle(40+x, 20+y, 5, U8G2_DRAW_ALL);
    }
  }
  u8g2.drawCircle(30+x, 30+y, 20, U8G2_DRAW_LOWER_LEFT);
  u8g2.drawCircle(30+x, 30+y, 20, U8G2_DRAW_LOWER_RIGHT);
  u8g2.drawCircle(30+x, 31+y, 20, U8G2_DRAW_LOWER_LEFT);
  u8g2.drawCircle(30+x, 31+y, 20, U8G2_DRAW_LOWER_RIGHT);
  u8g2.drawCircle(30+x, 32+y, 20, U8G2_DRAW_LOWER_LEFT);
  u8g2.drawCircle(30+x, 32+y, 20, U8G2_DRAW_LOWER_RIGHT);
}
#endif


bool hour_button_debounce(void)
{
  bool  flag = false;
  int   butt_raw;
  static unsigned long debounce_time;
  static int butt_state_last = HIGH;
  static int butt_state = HIGH;
  static bool butt_debouncing = false;
  
  butt_raw = digitalRead(HOUR_BUTTON);
  
  if (butt_raw != butt_state_last) {
    debounce_time = millis();
    butt_debouncing = true;
    butt_state_last = butt_raw;
  }
  
  if (butt_debouncing && ((millis() - debounce_time) > debounceDelay)) {
    butt_debouncing = false;
    flag = true;
    
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:

    // if the button state has changed:
    if (butt_raw != butt_state) {
      butt_state = butt_raw;

      // only toggle the LED if the new button state is HIGH
//      if (l_butt_state == LOW) {
//        flag = true;
//      }
    }
//    l_butt_state_last = l_butt_raw;
  }

  return flag;
}


bool min_button_debounce(void)
{
  bool flag = false;
  int   butt_raw;
  static unsigned long debounce_time;
  static int butt_state_last = HIGH;
  static int butt_state = HIGH;
  static bool butt_debouncing = false;
  
  butt_raw = digitalRead(MIN_BUTTON);
  
  if (butt_raw != butt_state_last) {
    debounce_time = millis();
    butt_debouncing = true;
    butt_state_last = butt_raw;
  }
  
  if (butt_debouncing && ((millis() - debounce_time) > debounceDelay)) {
    butt_debouncing = false;
    flag = true;
    
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:

    // if the button state has changed:
    if (butt_raw != butt_state) {
      butt_state = butt_raw;

      // only toggle the LED if the new button state is HIGH
//      if (r_butt_state == LOW) {
//        flag = true;
//      }
    }
//    r_butt_state_last = r_butt_raw;
  }

  return flag;
}


