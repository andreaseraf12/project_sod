/* Basic Multi Threading Arduino Example
   This example code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
// Please read file README.md in the folder containing this example./*
#include "RTClib.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_BMP280.h>
#include <Wire.h>

// Define two tasks for reading and writing from and to the serial port.
void TaskSerial(void *pvParameters);
void Task_Sensors(void *pvParameters);
void Task_Sync(void *pvParameters);

// Define Queue handle
QueueHandle_t QueueHandle;
const int QueueElementSize = 10;

Adafruit_BMP280 bmp;                         //Inizializzazione dell'oggetto BMP280
Adafruit_SSD1306 display(128, 64, &Wire, -1);
RTC_PCF8523 rtc; //inizializzazione dell'oggetto rtc


typedef struct{
  float temperatura;
  float pressione;
  String time_stamp;
} message_t;

// The setup function runs once when you press reset or power on the board.
void setup() {
  // Initialize serial communication at 115200 bits per second:
  Serial.begin(115200);
  while(!Serial){delay(10);}

    //BMP begin
  unsigned status;
  status = bmp.begin();
  if (!status) {
    Serial.println("Errore BMP");
    while (1) delay(10);
  }
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     // Modalità operativa
                  Adafruit_BMP280::SAMPLING_X2,     // Campionamento della temperatura 
                  Adafruit_BMP280::SAMPLING_X16,    // Campionamento della pressione 
                  Adafruit_BMP280::FILTER_X16,      // Filtraggio dei segnali
                  Adafruit_BMP280::STANDBY_MS_500); // Tempo di stand-by
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

  // Create the queue which will have <QueueElementSize> number of elements, each of size `message_t` and pass the address to <QueueHandle>.
  QueueHandle = xQueueCreate(QueueElementSize, sizeof(message_t));

  // Check if the queue was successfully created
  if(QueueHandle == NULL){
    Serial.println("Queue could not be created. Halt.");
    while(1) delay(1000); // Halt at this point as is not possible to continue
  }

  // Set up two tasks to run independently.
  xTaskCreate(
    TaskSerial
    ,  "TaskSerial" // A name just for humans
    ,  2048        // The stack size can be checked by calling `uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);`
    ,  NULL        // No parameter is used
    ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL // Task handle is not used here
    );

  xTaskCreate(
    Task_Sensors
    ,  "Task_Sensors"
    ,  2048  // Stack size
    ,  NULL  // No parameter is used
    ,  1  // Priority
    ,  NULL // Task handle is not used here
    );

    xTaskCreate(
    Task_Sync
    ,  "TaskSync" // A name just for humans
    ,  2048        // The stack size can be checked by calling `uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);`
    ,  NULL        // No parameter is used
    ,  0  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL // Task handle is not used here
    );

  
  
}

void loop(){
  // Loop is free to do any other work

  delay(1000); // While not being used yield the CPU to other tasks
}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/

void TaskSerial(void *pvParameters){  // This is a task.
  message_t message;
  for (;;){ // A Task shall never return or exit.
    // One approach would be to poll the function (uxQueueMessagesWaiting(QueueHandle) and call delay if nothing is waiting.
    // The other approach is to use infinite time to wait defined by constant `portMAX_DELAY`:
    if(QueueHandle != NULL){ // Sanity check just to make sure the queue actually exists
      int ret = xQueueReceive(QueueHandle, &message, portMAX_DELAY);
      if(ret == pdPASS){
       
     
        
        Serial.println(message.time_stamp);
        Serial.print("Temperatura: ");
        Serial.println(message.temperatura);
        Serial.print("Pressione: ");
        Serial.println(message.pressione);
        Serial.println("RPM:");
        //Serial.print(message.rpm);
        display.clearDisplay();
        //Setto il colore del testo a "bianco"
        display.setTextColor( WHITE);
        //Setto dimensione del testo
        display.setTextSize(1);
        //Sposto il cursore a metà altezza del display
        display.setCursor(0, 0);
        // The message was successfully received - send it back to Serial port and "Echo: "
        display.print("Temperatura: ");
        display.print(message.temperatura);
        display.println(" *C");
        display.print("Pressione: ");
        display.print(message.pressione);
        display.println(" atm");
        display.print("RPM:");
        //display.print(message.rpm);
        display.display();
        
        //Serial.print("Data: ");
        //Serial.println(message.time_stamp);
        // The item is queued by copy, not by reference, so lets free the buffer after use.
      }else if(ret == pdFALSE){
        Serial.println("The `TaskWriteToSerial` was unable to receive data from the Queue");
      }
    } // Sanity check
  } // Infinite loop
}

void Task_Sensors(void *pvParameters){  // This is a task.
  message_t message;
  for (;;){
   
    DateTime now = rtc.now();
    message.time_stamp = (String)now.year()+"/"+ (String)now.month()+ "/" +(String)now.day()+"_"+(String)now.hour()+":"+(String)now.minute()+":"+(String)now.second();
    message.temperatura = bmp.readTemperature();    //leggo temperatura
    message.pressione = bmp.readPressure()*9.8692*pow(10,-6);         //leggo pressione

    xQueueSend(QueueHandle, &message, portMAX_DELAY);
  delay(100);
  } // Infinite loop
}

void Task_Sync(void* pvParameters) {
  message_t message;
  for(;;){
    //verifico se ci sono dati sulla seriale
    if (Serial.available() > 0) {                                       
    // Leggi un byte dalla porta seriale
      char carattere_ricevuto = Serial.read();
       // Verifica se il carattere ricevuto è "1"
     if (carattere_ricevuto == '1') {
           // Esegui l'azione desiderata quando ricevi il carattere "1"
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
        Serial.println("Sincronizzazione effettuata");
        rtc.start();
        delay(100);
      }
    }
  }
}
