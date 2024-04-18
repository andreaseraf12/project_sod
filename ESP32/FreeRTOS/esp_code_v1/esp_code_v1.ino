/* Basic Multi Threading Arduino Example
   This example code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
// Please read file README.md in the folder containing this example./*

#include <Adafruit_BMP280.h>
#include <Wire.h>

#define MAX_LINE_LENGTH (64)
#define T_TASK 3100

// Define two tasks for reading and writing from and to the serial port.
void TaskSerial(void *pvParameters);
void Task_BMP_Temp(void *pvParameters);

// Define Queue handle
QueueHandle_t QueueHandle;
const int QueueElementSize = 10;

Adafruit_BMP280 bmp;                         //Inizializzazione dell'oggetto BMP280


typedef struct{
  float temperatura;
  float pressione;
  //String time_stamp;
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
    Task_BMP_Temp
    ,  "Task_BMP_Temp"
    ,  2048  // Stack size
    ,  NULL  // No parameter is used
    ,  1  // Priority
    ,  NULL // Task handle is not used here
    );

  // Now the task scheduler, which takes over control of scheduling individual tasks, is automatically started.
  Serial.printf("\nAnything you write will return as echo.\nMaximum line length is %d characters (+ terminating '0').\nAnything longer will be sent as a separate line.\n\n", MAX_LINE_LENGTH-1);
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
        // The message was successfully received - send it back to Serial port and "Echo: "
        Serial.print("Temperatura: ");
        Serial.println(message.temperatura);
        Serial.print("Pressione: ");
        Serial.println(message.pressione);
        //Serial.print("Data: ");
        //Serial.println(message.time_stamp);
        // The item is queued by copy, not by reference, so lets free the buffer after use.
      }else if(ret == pdFALSE){
        Serial.println("The `TaskWriteToSerial` was unable to receive data from the Queue");
      }
    } // Sanity check
  } // Infinite loop
}

void Task_BMP_Temp(void *pvParameters){  // This is a task.
  message_t message;
  for (;;){
    message.temperatura = bmp.readTemperature();    //leggo temperatura
    message.pressione = bmp.readPressure();         //leggo pressione

    xQueueSend(QueueHandle, &message, portMAX_DELAY);
  delay(100);
  } // Infinite loop
}
