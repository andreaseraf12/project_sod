/**************************************************************************/
/*
  Countdown Timer using a PCF8523 RTC connected via I2C and Wire lib
  with the INT/SQW pin wired to an interrupt-capable input.

  According to the data sheet, the PCF8523 can run countdown timers
  from 244 microseconds to 10.625 days:
  https://www.nxp.com/docs/en/data-sheet/PCF8523.pdf#page=34

  This sketch sets a countdown timer, and executes code when it reaches 0,
  then blinks the built-in LED like BlinkWithoutDelay, but without millis()!

  NOTE:
  You must connect the PCF8523's interrupt pin to your Arduino or other
  microcontroller on an input pin that can handle interrupts, and that has a
  pullup resistor. The pin will be briefly pulled low each time the countdown
  reaches 0. This example will not work without the interrupt pin connected!

  On Adafruit breakout boards, the interrupt pin is labeled 'INT' or 'SQW'.
*/
/**************************************************************************/

#include "RTClib.h"

RTC_PCF8523 rtc;

// Input pin with interrupt capability
// const int timerInterruptPin = 2;  // Most Arduinos
#define timerInterruptPin 0  // ESP32 PIN
#define PIN_TACHO 16

// Counter e Interrupt trigger per contare 1 secondo in RTC 
volatile bool countdownInterruptTriggered = false;
volatile int numCountdownInterrupts = 0;

// Counter per il tacho
volatile int counter = 0;

void setup () {
  Serial.begin(115200);

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }


  // Set the pin attached to PCF8523 INT to be an input with pullup to HIGH.
  // The PCF8523 interrupt pin will briefly pull it LOW at the end of a given
  // countdown period, then it will be released to be pulled HIGH again.

  // Abilitare la resistenza di pull-up sui pin 0 e 16 (SQW e TACHO)
  pinMode(timerInterruptPin, INPUT_PULLUP);
  pinMode(PIN_TACHO, INPUT_PULLUP);

  Serial.println(F("\nStarting PCF8523 Countdown Timer example."));
  Serial.print(F("Configured to expect PCF8523 INT/SQW pin connected to input pin: "));
  Serial.println(timerInterruptPin);
  Serial.println(F("This example will not work without the interrupt pin connected!\n\n"));


  //Calibrazione RTC per la data 
  if (! rtc.initialized() || rtc.lostPower()) {
    Serial.println("RTC is NOT initialized, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
  // Timer configuration is not cleared on an RTC reset due to battery backup!
  //rtc.deconfigureAllTimers();

  // These are the PCF8523's built-in "Timer Source Clock Frequencies".
  // They are predefined time periods you choose as your base unit of time,
  // depending on the length of countdown timer you need.
  // The minimum length of your countdown is 1 time period.
  // The maximum length of your countdown is 255 time periods.
  //
  // PCF8523_FrequencyHour   = 1 hour, max 10.625 days (255 hours)
  // PCF8523_FrequencyMinute = 1 minute, max 4.25 hours
  // PCF8523_FrequencySecond = 1 second, max 4.25 minutes
  // PCF8523_Frequency64Hz   = 1/64 of a second (15.625 milliseconds), max 3.984 seconds
  // PCF8523_Frequency4kHz   = 1/4096 of a second (244 microseconds), max 62.256 milliseconds
  //
  //
  // These are the PCF8523's optional 'Low Pulse Widths' of time the interrupt
  // pin is held LOW at the end of every countdown (frequency 64Hz or longer).
  //
  // PCF8523_LowPulse3x64Hz  =  46.875 ms   3/64ths second (default)
  // PCF8523_LowPulse4x64Hz  =  62.500 ms   4/64ths second
  // PCF8523_LowPulse5x64Hz  =  78.125 ms   5/64ths second
  // PCF8523_LowPulse6x64Hz  =  93.750 ms   6/64ths second
  // PCF8523_LowPulse8x64Hz  = 125.000 ms   8/64ths second
  // PCF8523_LowPulse10x64Hz = 156.250 ms  10/64ths second
  // PCF8523_LowPulse12x64Hz = 187.500 ms  12/64ths second
  // PCF8523_LowPulse14x64Hz = 218.750 ms  14/64ths second
  //
  //
  // Uncomment an example below:

  // rtc.enableCountdownTimer(PCF8523_FrequencyHour, 24);    // 1 day
  // rtc.enableCountdownTimer(PCF8523_FrequencyMinute, 150); // 2.5 hours
  //rtc.enableCountdownTimer(PCF8523_FrequencySecond, 10);  // 10 seconds
  // rtc.enableCountdownTimer(PCF8523_Frequency64Hz, 32);    // 1/2 second
     rtc.enableCountdownTimer(PCF8523_Frequency64Hz, 64, PCF8523_LowPulse8x64Hz);    // 1 second
  // rtc.enableCountdownTimer(PCF8523_Frequency64Hz, 16);    // 1/4 second
  // rtc.enableCountdownTimer(PCF8523_Frequency4kHz, 205);   // 50 milliseconds

  attachInterrupt(digitalPinToInterrupt(timerInterruptPin), countdownOver, FALLING);
  attachInterrupt(digitalPinToInterrupt(PIN_TACHO), countPulses, FALLING);
  
}

// Triggered by the PCF8523 Countdown Timer interrupt at the end of a countdown
// period. Meanwhile, the PCF8523 immediately starts the countdown again.

//Funzione che conta gli impulsi da parte di RTC
void countdownOver () {
  // Set a flag to run code in the loop():
  countdownInterruptTriggered = true;
  numCountdownInterrupts++;
}

//Funzione che conta gli impulsi da parte di TACHO  
void countPulses() {
  counter ++;
}


void loop () {

  if (countdownInterruptTriggered && numCountdownInterrupts == 1) {
    DateTime now = rtc.now();
    String time_stamp = (String)now.year()+"/"+ (String)now.month()+ "/" +(String)now.day()+"_"+(String)now.hour()+":"+(String)now.minute()+":"+(String)now.second();
    Serial.println(time_stamp);
    
    //detachInterrupt(digitalPinToInterrupt(PIN_TACHO));
    //Calcolo RPM
    float rpm = counter * 60 / 2;
    Serial.println(rpm);
    counter = 0;
    //attachInterrupt(digitalPinToInterrupt(PIN_TACHO), countPulses, FALLING);

    countdownInterruptTriggered = false; // don't come in here again
    numCountdownInterrupts = 0;
    
  }
  //delay(100);
  }

  
