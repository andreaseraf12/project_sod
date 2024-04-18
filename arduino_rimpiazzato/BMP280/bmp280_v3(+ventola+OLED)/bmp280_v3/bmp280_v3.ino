/***************************************************************************
  This is a library for the BMP280 humidity, temperature & pressure sensor

  Designed specifically to work with the Adafruit BMP280 Breakout
  ----> http://www.adafruit.com/products/2651

  These sensors use I2C or SPI to communicate, 2 or 4 pins are required
  to interface.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit andopen-source hardware by purchasing products
  from Adafruit!

  Written by Limor Fried & Kevin Townsend for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ***************************************************************************/

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define BMP_SCK  (13)
#define BMP_MISO (12)
#define BMP_MOSI (11)
#define BMP_CS   (10)

/*
   Definisco le dimensioni del display, serviranno per creare l'istanza del display
   e magari riutilizzarle nel codice qualora dovessero servirmi queste informazioni
*/

// Creo istanza del display
Adafruit_SSD1306 display(128, 64);

Adafruit_BMP280 bmp; // I2C
//Adafruit_BMP280 bmp(BMP_CS); // hardware SPI
//Adafruit_BMP280 bmp(BMP_CS, BMP_MOSI, BMP_MISO,  BMP_SCK);

float soglia=24.0;
int fanSpeed=0;
int analogpin=A3;
float temperatura, pressione;

void setup() {
  //OLED
   //Inizializzo il display OLED all'indirizzo 0x3D (il tuo potrebbe essere diverso)
  if (!display.begin( SSD1306_SWITCHCAPVCC, 0x3D)) {
    /*
      Se non sono riuscito ad inizializzare il display
      creo un loop infinito ed impedisco al programma di andare avanti
    */
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
  pinMode(analogpin, INPUT);
  //SERIALE
  Serial.begin(9600);
  //BMP
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

  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
  
}

void loop() {
    Serial.print(F("Temperature = "));
    Serial.print(temperatura = bmp.readTemperature());
    Serial.println(" *C");

    Serial.print(F("Pressure = "));
    pressione=bmp.readPressure()*9.86923*pow(10,-6);
    Serial.print(bmp.readPressure());
    Serial.println(" Pa");
    
   /* Serial.print(F("Approx altitude = "));
    Serial.print(bmp.readAltitude(1013.25)); /* Adjusted to local forecast! 
    Serial.println(" m");*/

    Serial.println();
    
    int rpm=analogRead(analogpin);
  //float rpm2=(rpm*60)/2;
  Serial.println(rpm);
 
  if(temperatura  >= soglia) {                             // if temperature is higher than minimum temp 
      fanSpeed = map(temperatura, soglia, 38, 0, 255);    // the actual speed of fan 
      analogWrite(6, fanSpeed);                          // spin the fan at the fanSpeed speed 
      stampa_OLED();
   } 
  else{                                     // if temp is lower than minimum temp 
      fanSpeed = 0;                        // fan is not spinning 
      digitalWrite(6, LOW); 
      stampa_OLED();
   } 

    
    delay(500);
}


  void stampa_OLED(){
    //Ripulisco il buffer
  display.clearDisplay();
  //Setto il colore del testo a "bianco"
  display.setTextColor( WHITE);
  //Setto dimensione del testo
  display.setTextSize(2);
  //Sposto il cursore a metà altezza del display
  display.setCursor(0, 0);
  //Stampo una scritta
  display.print(temperatura);
  display.print((char)247);
  display.println("C");
  display.print(pressione);
  display.println("atm");
  //La mando in stampa
  display.display();
  }
  
