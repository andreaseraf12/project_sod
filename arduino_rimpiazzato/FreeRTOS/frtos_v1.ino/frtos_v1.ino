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


  // reserve 1 bytes for the inputString:
  inputString.reserve(1);
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

void _taskSyncRTC(void* param){
    /*Serial.println("task_sync");
    while (1) {
     // Se sulla linea seriale si LEGGE il valore 1 allora bisogna rilasciare il semaforo
    // Pulizia linea seriale (forse da fare)
    // Lettura linea seriale
    // Verificare la condizione dell'if ed eventualmente rilasciare il semaforo e pulire la seriale
    // Serial.flushInput();
    // Serial.end()
    if (Serial.available() > 0){
      char flag = Serial.read();
      if(flag == '1'){
        Serial.println("Sincronizzazione");
        loop_SyncRTC();
        delay(10);
        }
    }
      vTaskDelay(1100 / portTICK_PERIOD_MS);
      }*/
      
      while (1) {
        Serial.println("task_sync");
     // Verifica se ci sono dati disponibili sulla porta seriale
    if (Serial.available() > 0) {                                      
    // Leggi un byte dalla porta seriale
      char receivedChar = Serial.read();
       Serial.println("Task no. 1!");
    // Verifica se il carattere ricevuto è "1"
      if (receivedChar == '1') {
      // Esegui l'azione desiderata quando ricevi il carattere "1"
        Serial.println("Ricevuto il carattere UNO. Sei dentro if");
      // Aggiungi qui le azioni che desideri eseguire
      }

    // Stampa il byte ricevuto
    Serial.println("Hai ricevuto il seguente carattere: ");
    Serial.println(receivedChar);
    Serial.println();
    }
  vTaskDelay(1100 / portTICK_PERIOD_MS);
  }
}

void _taskReadSensors(void* param){
    
    while (1) {
      Serial.println("task_Sensor");
      //data = loop_rtcTime();
      //Serial.println(data);
      vTaskDelay(100000 / portTICK_PERIOD_MS);
    }
  
  }

  void _taskOLED(void* param){
    while (1) {
      Serial.println("task_OLED");
      vTaskDelay(10000/ portTICK_PERIOD_MS);}
    }

void loop() {}
