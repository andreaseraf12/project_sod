//FreeRTOS library:
#include <Arduino_FreeRTOS.h>

void setup() {
  Serial.begin(9600);
  
  xTaskCreate(Task_1, "Task no. 1!", 100, NULL, 1, NULL);
  xTaskCreate(Task_2, "Task no.  2!", 100, NULL, 2, NULL);
  xTaskCreate(Task_3, "Task no.  3!", 100, NULL, 3, NULL);
  
}

//The following function is Task1. We display the task label on Serial monitor.
static void Task_1(void* pvParameters) {
  while (1) {
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

//Task 2
static void Task_2(void* pvParameters) {
  while (1) {

    Serial.println("Task no. 2!");
    Serial.println();
    vTaskDelay(1100 / portTICK_PERIOD_MS);
  }
}

//Task 3
static void Task_3(void* pvParameters) {
  while (1) {
    Serial.println("Task no. 3!");
    Serial.println();
    vTaskDelay(1200 / portTICK_PERIOD_MS);
  }
}

void loop() {}
