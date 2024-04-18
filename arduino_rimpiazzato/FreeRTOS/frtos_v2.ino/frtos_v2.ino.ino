//FreeRTOS library:
#include <Arduino_FreeRTOS.h>
#include "RTClib.h"

String inputString = "";         // a String to hold incoming data
bool stringComplete = false;  // whether the string is complete
float temperatura, pressione, vel_ventola; // inizializzazione variabili globali
String data;
RTC_PCF8523 rtc; //inizializzazione dell'oggetto rtc

//Inizializzazione dei task
void setup() {
  //Inizializzazione comunicazione seriale per Sync
  Serial.begin(9600);
  Serial.flush();
  xTaskCreate(_taskSyncRTC, "SyncRTC" , 128, NULL, -1, NULL );
  xTaskCreate(_taskReadSensors, "ReadSensors" , 128, NULL, 0, NULL );
  xTaskCreate(_taskOLED, "OLED" , 128, NULL, 1, NULL );
 
  pinMode(LED_BUILTIN, OUTPUT);

  // Inizializzazione modulo RTC
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1) delay(10);
  }

  if (! rtc.initialized() || rtc.lostPower()) {
    Serial.println("RTC is NOT initialized, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
    rtc.start();

  float drift = 43; // seconds plus or minus over oservation period - set to 0 to cancel previous calibration.
  float period_sec = (7 * 86400);  // total obsevation period in seconds (86400 = seconds in 1 day:  7 days = (7 * 86400) seconds )
  float deviation_ppm = (drift / period_sec * 1000000); //  deviation in parts per million (μs)
  float drift_unit = 4.069; //For corrections every min the drift_unit is 4.069 ppm (use with offset mode PCF8523_OneMinute)
  int offset = round(deviation_ppm / drift_unit);
 
}


//The following function is Task1. We display the task label on Serial monitor.
static void _taskSyncRTC(void* pvParameters) {
  while (1) {
     // Verifica se ci sono dati disponibili sulla porta seriale
    if (Serial.available() > 0) {                                       //
    // Leggi un byte dalla porta seriale
      char receivedChar = Serial.read();
    // Verifica se il carattere ricevuto è "1"
      if (receivedChar == '1') {
            digitalWrite(LED_BUILTIN, HIGH);   
            delay(1300);                       
            digitalWrite(LED_BUILTIN, LOW);
      // Esegui l'azione desiderata quando ricevi il carattere "1"
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
        Serial.println("Ricevuto il carattere UNO. Sincronizzazione effettuata");
        rtc.start();
        delay(100);
      }
    }
  vTaskDelay(1100 / portTICK_PERIOD_MS); // serve per dare il tempo al SO di effettuare gli altri task
  }
}

//Task 2
static void _taskReadSensors(void* pvParameters) {
  while (1) {
    Serial.println("task_Sensor");
    DateTime now = rtc.now();
    data = (String)now.year()+"/"+ (String)now.month()+ "/" +(String)now.day()+"_";
    data = data +(String)now.hour()+":"+(String)now.minute()+":"+(String)now.second();
    Serial.println(data);
    vTaskDelay(1100 / portTICK_PERIOD_MS);
    }
}

//Task 3
void _taskOLED(void* param){
    while (1) {
      Serial.println("task_OLED");
      vTaskDelay(1100/ portTICK_PERIOD_MS);}
    }

void loop() {}
