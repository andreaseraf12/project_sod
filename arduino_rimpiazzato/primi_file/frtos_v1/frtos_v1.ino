#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include "RTClib.h"
//#include "serial_syncRTC_v1.h"

//#include "rtc_timegenerator_v2.h"

String inputString = "";         // a String to hold incoming data
bool stringComplete = false;  // whether the string is complete
float temperatura, pressione, vel_ventola; // inizializzazione variabili globali
String data;
RTC_PCF8523 rtc; //inizializzazione dell'oggetto rtc

// Dichiarazione del semaforo
SemaphoreHandle_t sem;

//Inizializzazione dei task
void setup() {
  xTaskCreate(_taskSyncRTC, "SyncRTC" , 128, NULL, -1, NULL );
  xTaskCreate(_taskReadSensors, "ReadSensors" , 128, NULL, 0, NULL );
  xTaskCreate(_taskOLED, "OLED" , 128, NULL, 1, NULL );
  //Inizializzazione del semaforo
  sem = xSemaphoreCreateBinary();

  //Inizializzazione comunicazione seriale per Sync
  Serial.begin(57600);
  Serial.flush();
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

void loop() {}

void _taskSyncRTC(){
    xSemaphoreTake(sem, portMAX_DELAY);
    Serial.println("task_sync");
     // Se sulla linea seriale si LEGGE il valore 1 allora bisogna rilasciare il semaforo
    // Pulizia linea seriale (forse da fare)
    // Lettura linea seriale
    // Verificare la condizione dell'if ed eventualmente rilasciare il semaforo e pulire la seriale
    // Serial.flushInput();
    // Serial.end()
    /*if (xSemaphoreTake(sem, portMAX_DELAY) == pdPASS) {
      int flag = 0;
      flag = Serial.read();
      if(flag == 1){
        Serial.println("Sincronizzazione");
        loop_SyncRTC();
        //Serial.flush();
        delay(10);
        }
      xSemaphoreGive(sem);
      vTaskDelay(pdMS_TO_TICKS(1000));}*/
      xSemaphoreGive(sem);
      vTaskDelay(pdMS_TO_TICKS(1000));
    delay(1000);
}

void _taskReadSensors(){
    Serial.println("task_Sensor");
  if (xSemaphoreTake(sem, portMAX_DELAY) == pdPASS){
  
    //data = loop_rtcTime();
    Serial.println("Ciao");
    //Serial.flush();
    xSemaphoreGive(sem);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
  delay(1000);
  }

  void _taskOLED(){}
