#include <Arduino_FreeRTOS.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include "RTClib.h"
#include <queue.h>

#define ANALOG_PIN A3
#define SOGLIA_TEMP 25.0
#define PIN_PWM 6
#define T_TASK 3100

String inputString = "";         // a String to hold incoming data
bool stringComplete = false;  // whether the string is complete

// Define a struct
struct data_struct {
  float temperatura;
  float pressione;
  String time_stamp;
};

QueueHandle_t structQueue;
//Adafruit_SSD1306 display(128, 64, &Wire, -1);//istanza del display
Adafruit_BMP280 bmp;                         //Inizializzazione dell'oggetto BMP280
RTC_PCF8523 rtc;                            //inizializzazione dell'oggetto rtc

void setup() {
  // Init Arduino serial
  Serial.begin(9600);
  // Wait for serial port to connect
  while (!Serial) delay(100);

  //BMP e RTC begin
  unsigned status;
  status = bmp.begin();
  if (!status) {
    Serial.println("Errore BMP");
    while (1) delay(10);
  }

   if (!rtc.begin()) {
    Serial.println("Errore RTC");
    while (1) delay(10);
  }
  if (! rtc.initialized() || rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
    rtc.start();
    
  /**
   * Create a queue.
   * https://www.freertos.org/a00116.html
   */
  structQueue = xQueueCreate(10, // Queue length
                              sizeof(struct data_struct) // Queue item size
                              );
  
  if (structQueue != NULL) {
    
    // Create task that consumes the queue if it was created.
    xTaskCreate(TaskSerial, // Task function
                "Serial", // A name just for humans
                128,  // This stack size can be checked & adjusted by reading the Stack Highwater
                NULL, 
                2, // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
                NULL);


    // Create task that publish data in the queue if it was created.
    xTaskCreate(Task_BMP_Temp, // Task function
                "Task_BMP_Temp", // Task name
                128,  // Stack size
                NULL, 
                1, // Priority
                NULL);

    // Create other task that publish data in the queue if it was created.
    
    
  }
}


 void loop() {}

//Task 1
void Task_BMP_Temp(void *pvParameters)
{
  (void) pvParameters;
  
  for (;;)
  {
    // Read the input on analog pin 0:
    struct data_struct msg;
    msg.temperatura = bmp.readTemperature();    //leggo temperatura
    msg.pressione = bmp.readPressure();         //leggo pressione
    DateTime now = rtc.now();                   //interrogo l'RTC
    ///converto in stringa la data
    msg.time_stamp=(String)now.hour()+":"+(String)now.minute()+":"+(String)now.second(); 
    /** 
     * Post an item on a queue.
     * https://www.freertos.org/a00117.html
     */
    xQueueSend(structQueue, &msg, portMAX_DELAY);

    // One tick delay (15ms) in between reads for stability
    vTaskDelay(50);
  }
}



//Task Seriale
void TaskSerial(void * pvParameters) {
  (void) pvParameters;

  
  for (;;) 
  {

    struct data_struct msg;

    /**
     * Read an item from a queue.
     * https://www.freertos.org/a00118.html
     */
    if (xQueueReceive(structQueue, &msg, portMAX_DELAY) == pdPASS) {
      Serial.print("Temperatura: ");
      Serial.println(msg.temperatura);
      Serial.print("Pressione: ");
      Serial.println(msg.pressione);
      Serial.print("Data: ");
      Serial.println(msg.time_stamp);
    }
  }
}
