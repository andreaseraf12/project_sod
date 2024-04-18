#include <Adafruit_BMP280.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Arduino_FreeRTOS.h>
#include "RTClib.h"

#define ANALOG_PIN A3
#define SOGLIA_TEMP 25.0
#define PIN_PWM 6
#define T_TASK 3100

String inputString = "";         // a String to hold incoming data
bool stringComplete = false;  // whether the string is complete
float temperatura, pressione; // inizializzazione variabili globali
String data;
int fanSpeed=0; 

//Adafruit_SSD1306 display(128, 64, &Wire, -1); //istanza del display
RTC_PCF8523 rtc; //inizializzazione dell'oggetto rtc
Adafruit_BMP280 bmp;                         //Inizializzazione dell'oggetto BMP280

//Inizializzazione dei task
void setup() {
  //Inizializzazione comunicazione seriale per Sync
  Serial.begin(9600);
  Serial.flush();
  xTaskCreate(_taskSyncRTC, "SyncRTC" , 128, NULL, -1, NULL );
  xTaskCreate(_taskReadSensors, "ReadSensors" , 128, NULL, 0, NULL );
  xTaskCreate(_taskOLED, "OLED" , 128, NULL, 1, NULL );
 
  pinMode(LED_BUILTIN, OUTPUT);
  
   //BMP
  bmp.begin();
   
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

}


//The following function is Task1. We display the task label on Serial monitor.
static void _taskSyncRTC(void* pvParameters) {
  while (1) {
    Serial.println("Task 1\n");
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
  vTaskDelay(T_TASK / portTICK_PERIOD_MS); // serve per dare il tempo al SO di effettuare gli altri task
  }
}

//Task 2
static void _taskReadSensors(void* pvParameters) {
  while (1) {
    Serial.println("Task 2");
    DateTime now = rtc.now();
    data = (String)now.year()+"/"+ (String)now.month()+ "/" +(String)now.day()+"_"+(String)now.hour()+":"+(String)now.minute()+":"+(String)now.second();
    Serial.println("data");
    Serial.print(F("Temperature = "));
    Serial.print(temperatura = bmp.readTemperature());
    Serial.println(" *C");

    /*Serial.print(F("Pressure = "));
    Serial.print(pressione=bmp.readPressure()*9.86923*pow(10,-6));
    Serial.println(" Pa\n");*/
        
       
        if(temperatura  >= SOGLIA_TEMP) {                            
          fanSpeed = map(temperatura, SOGLIA_TEMP, 38, 0, 255);    
          analogWrite(6, fanSpeed);
          Serial.println("sopra soglia\n");                           
          //stampa_OLED();
   } 
        else{                                     
          fanSpeed = 0;                        
          digitalWrite(6, LOW); 
          Serial.println("sotto soglia\n");                           
        //stampa_OLED();
   }
   vTaskDelay(T_TASK/ portTICK_PERIOD_MS);
    }
}

//Task 3
void _taskOLED(void* param){
    while (1) {
      Serial.println("Task 3\n");
      vTaskDelay(T_TASK/ portTICK_PERIOD_MS);
      }
    }

void loop() {}
