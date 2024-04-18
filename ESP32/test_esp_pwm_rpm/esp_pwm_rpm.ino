#define CANALE 0
#define RISOLUZIONE_PWM 8
#define FREQUENZA 5000
#define PIN_PWM 17

#include <Adafruit_BMP280.h>

//BMP280
Adafruit_BMP280 bmp; // I2C

float soglia=21.0;
int fanSpeed=0;
float temperatura;

//RPM
const int tachoPin = 16;
volatile unsigned long counter = 0;

void countPulses() {
  counter ++;
}

void setup() {
    //BMP
    Serial.begin(9600);
    while ( !Serial ) delay(100);   // wait for native usb
    Serial.println(F("BMP280 test"));
    unsigned status;
    //status = bmp.begin(BMP280_ADDRESS_ALT, BMP280_CHIPID);
    status = bmp.begin();
    if (!status) {
        Serial.println(F("Could not find a valid BMP280 sensor, check wiring or "
                        "try a different address!"));
        /*Serial.print("SensorID was: 0x"); Serial.println(bmp.sensorID(),16);
        Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
        Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
        Serial.print("        ID of 0x60 represents a BME 280.\n");
        Serial.print("        ID of 0x61 represents a BME 680.\n");*/
        while (1) delay(10);
    }


    ledcSetup(CANALE, FREQUENZA, RISOLUZIONE_PWM);
    ledcAttachPin(PIN_PWM, CANALE);

    //RPM
    attachInterrupt(digitalPinToInterrupt(tachoPin), countPulses, FALLING);

    
}

void loop(){
    Serial.print(F("Temperature = "));
    Serial.print(temperatura = bmp.readTemperature());
    Serial.println(" *C");

    

     if(temperatura  < soglia) {           // if temp is lower than minimum temp 
      fanSpeed = 0;                        // fan is not spinning 
      ledcWrite(CANALE, fanSpeed); 
   } 
   if(temperatura  >= soglia) {                           // if temperature is higher than minimum temp 
      fanSpeed = map(temperatura, soglia, 28, 0, 255);    // the actual speed of fan 
      ledcWrite(CANALE, fanSpeed);                          // spin the fan at the fanSpeed speed 
   } 
    
    long  rpm = counter * 60 / 2;
    Serial.print("RPM:");
    Serial.println(rpm);
    counter = 0;
    delay(1000);

}
