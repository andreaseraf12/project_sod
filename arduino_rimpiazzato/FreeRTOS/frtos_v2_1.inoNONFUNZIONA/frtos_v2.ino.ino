//FreeRTOS library:
#include <Arduino_FreeRTOS.h>
#include "RTClib.h"
#include <Adafruit_BMP280.h>
#include <Adafruit_SSD1306.h>


Adafruit_SSD1306 display(128, 64); // Creo istanza del display
RTC_PCF8523 rtc; //inizializzazione dell'oggetto rtc
Adafruit_BMP280 bmp; //inizializzazione BMP

String inputString = "";         // a String to hold incoming data
String data;
float soglia=24.0;
int fanSpeed=0;
float temperatura, pressione;

void setup() {
   //Inizializzazione comunicazione seriale per Sync
  Serial.begin(9600);
  Serial.flush();
    //OLED
   //Inizializzo il display OLED all'indirizzo 0x3D
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) {
    Serial.println("Errore OLED");
    while (true);
  }
    // OLED Pulisco il buffer
  display.clearDisplay();
  // OLED Applico la pulizia al display
  display.display();
  //PWM
  pinMode(6, OUTPUT);
  //RPM
  pinMode(A3, INPUT);
  
  //BMP
  /*while ( !Serial ) delay(100);   // wait for native usb
  Serial.println(F("BMP280 test"));
  unsigned status;
  status = bmp.begin();
  if (!status) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring or "
                      "try a different address!"));
    while (1) delay(10);
  }*/
  // Default settings from datasheet. 
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
  
  xTaskCreate(_taskSyncRTC, "SyncRTC" , 128, NULL, -1, NULL );
//  xTaskCreate(_taskReadSensors, "ReadSensors" , 128, NULL, 0, NULL );
//  xTaskCreate(_taskOLED, "OLED" , 128, NULL, 1, NULL );
 
  //pinMode(LED_BUILTIN, OUTPUT);

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


//The following function is Task1
static void _taskSyncRTC(void* pvParameters) {
  while (1) {
     // Verifica se ci sono dati disponibili sulla porta seriale
    if (Serial.available() > 0) {                                       
    // Leggi un byte dalla porta seriale
      char receivedChar = Serial.read();
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
      Serial.println("Ricevuto il carattere UNO. Sincronizzazione effettuata");
    // Verifica se il carattere ricevuto è "1"
     /* if (receivedChar == '1') {
           // Esegui l'azione desiderata quando ricevi il carattere "1"
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
        Serial.println("Ricevuto il carattere UNO. Sincronizzazione effettuata");
        rtc.start();
        delay(100);
      }*/
    }
  vTaskDelay(1100 / portTICK_PERIOD_MS); // serve per dare il tempo al SO di effettuare gli altri task
  }
}
/*
//Task 2
static void _taskReadSensors(void* pvParameters) {
  while (1) {
    //RTC
    DateTime now = rtc.now();
    data = /*(String)now.year()+"/"+ (String)now.month()+ "/" +(String)now.day()+"_"+(String)now.hour()+":"+(String)now.minute()+":"+(String)now.second();
        
    /*Print Seriale   (BMP280 + RTC)
    Serial.println(data); 
    Serial.print(F("Temperature = "));
    Serial.print(temperatura = bmp.readTemperature());
    //Serial.println(" *C");
    Serial.print(F("Pressure = "));
    Serial.print(pressione=bmp.readPressure()*9.86923*pow(10,-6));
    //Serial.println(" atm");
    Serial.println();*/
    /*
    vTaskDelay(1100 / portTICK_PERIOD_MS);
    }
}*/
/*
//Task 3
void _taskOLED(void* param){
    while (1) {
      Serial.println("TASK_OLED");
      vTaskDelay(1100/ portTICK_PERIOD_MS);}
    }*/

void loop() {}
