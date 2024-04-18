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

#define CANALE 0
#define RISOLUZIONE_PWM 8
#define FREQUENZA 5000
#define PIN_PWM 17

#define PIN_TACHO 16
#define SOGLIA 21
#define PIN_COUNTDOWNINT 0

//RPM
volatile bool countdownInterruptTriggered = false;
volatile int numCountdownInterrupts = 0;
volatile unsigned long counter = 0;
int fanSpeed=0;

// Funzione per contare gli impulsi da Tacho
void countPulses() {
  counter ++;
}

// Funzione per contare gli impulsi da parte di RTC per contare 1sec
void countdownOver () {
  // Set a flag to run code in the loop():
  countdownInterruptTriggered = true;
  numCountdownInterrupts++;
}


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
  long rpm;
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
    rtc.enableCountdownTimer(PCF8523_Frequency64Hz, 64, PCF8523_LowPulse8x64Hz);  
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

    //inizializzazione PWM 
    ledcSetup(CANALE, FREQUENZA, RISOLUZIONE_PWM);
    ledcAttachPin(PIN_PWM, CANALE); 
    ledcWrite(CANALE,0);

    //Associazionie degli interrupt ai pin e resistenze pull-up
    pinMode(PIN_COUNTDOWNINT, INPUT_PULLUP);
    pinMode(PIN_TACHO, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(PIN_TACHO), countPulses, FALLING); //RPM
    attachInterrupt(digitalPinToInterrupt(PIN_COUNTDOWNINT), countdownOver, FALLING); //RTC
  
}

void loop(){
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
       
     
        
        Serial.println(message.time_stamp+"#"+message.temperatura+"#"+message.pressione+"#"+message.rpm);
        /*Serial.print("#");
        Serial.print(message.temperatura);
        Serial.print("#");
        Serial.print(message.pressione);
        Serial.print("#");
        Serial.println(message.rpm);*/
        display.clearDisplay();
        //Setto il colore del testo a "bianco"
        display.setTextColor(WHITE);
        //Setto dimensione del testo
        display.setTextSize(1);
        //Sposto il cursore a metà altezza del display
        display.setCursor(0, 0);
        // The message was successfully received - send it back to Serial port and "Echo: "
        display.print("Temperatura: ");
        display.print(message.temperatura);
        display.println((char)247);
        display.println("C");
        display.print("Pressione: ");
        display.print(message.pressione);
        display.println(" atm");
        display.print("RPM:");
        display.print(message.rpm);
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
    delay(1000);
    DateTime now = rtc.now();
    message.time_stamp = (String)now.year()+"-"+ (String)now.month()+ "-" +(String)now.day()+" "+(String)now.hour()+":"+(String)now.minute()+":"+(String)now.second();
    message.temperatura = bmp.readTemperature();    //leggo temperatura
    message.pressione = bmp.readPressure()*9.8692*pow(10,-6); //leggo pressione 
   /*
   if(message.temperatura  >= SOGLIA) {                           // if temperature is higher than minimum temp 
      fanSpeed = map(message.temperatura, SOGLIA, 28, 0, 255);    // the actual speed of fan 
      //ledcWrite(CANALE, constrain(fanSpeed, 0, 255));  //funzione per forzare i valori dell'output di map() nel range [0,255]
      if(fanSpeed > 255) fanSpeed = 255;
      ledcWrite(CANALE, fanSpeed);                          // spin the fan at the fanSpeed speed 
      
   } 
    else {           // if temp is lower than minimum temp 
      fanSpeed = 0;                        // fan is not spinning 
      ledcWrite(CANALE, fanSpeed); 
   }*/
    fanSpeed = map(message.temperatura, SOGLIA, 28, 0, 255);
    ledcWrite(CANALE, constrain(fanSpeed, 0, 255));
   

   if(countdownInterruptTriggered && numCountdownInterrupts == 1){
    //detachInterrupt(digitalPinToInterrupt(PIN_TACHO));
    message.rpm = counter * 60 / 2;
    counter = 0;
    //attachInterrupt(digitalPinToInterrupt(PIN_TACHO), countPulses, FALLING);
    countdownInterruptTriggered = false; // don't come in here again
    numCountdownInterrupts = 0;
    }
    


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
