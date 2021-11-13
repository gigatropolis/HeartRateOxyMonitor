

#include <ArduinoBLE.h>
#include <Wire.h>
//#include "MAX30100_PulseOximeter.h"
 
#include "Wire.h"
#include "Adafruit_GFX.h"
#include "OakOLED.h"
#include "MAX30105.h"

MAX30105 particleSensor; // initialize MAX30102 with I2C

#define REPORTING_PERIOD_MS 1000
OakOLED oled;
int smokeA0 = 0;
 
//PulseOximeter pox;
 
uint32_t tsLastReport = 0;
int count = 0;

const unsigned char bitmap [] PROGMEM=
{
0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x18, 0x00, 0x0f, 0xe0, 0x7f, 0x00, 0x3f, 0xf9, 0xff, 0xc0,
0x7f, 0xf9, 0xff, 0xc0, 0x7f, 0xff, 0xff, 0xe0, 0x7f, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xf0,
0xff, 0xf7, 0xff, 0xf0, 0xff, 0xe7, 0xff, 0xf0, 0xff, 0xe7, 0xff, 0xf0, 0x7f, 0xdb, 0xff, 0xe0,
0x7f, 0x9b, 0xff, 0xe0, 0x00, 0x3b, 0xc0, 0x00, 0x3f, 0xf9, 0x9f, 0xc0, 0x3f, 0xfd, 0xbf, 0xc0,
0x1f, 0xfd, 0xbf, 0x80, 0x0f, 0xfd, 0x7f, 0x00, 0x07, 0xfe, 0x7e, 0x00, 0x03, 0xfe, 0xfc, 0x00,
0x01, 0xff, 0xf8, 0x00, 0x00, 0xff, 0xf0, 0x00, 0x00, 0x7f, 0xe0, 0x00, 0x00, 0x3f, 0xc0, 0x00,
0x00, 0x0f, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
 
void onBeatDetected()
{
  //Serial.println("Beat!");
  oled.drawBitmap( 60, 20, bitmap, 28, 28, 1);
  oled.display();
}
 
int Delay = 1000;
int connectDelay = 200;

BLEService customService("5505");
BLEUnsignedIntCharacteristic HeartRate ("2101", BLERead | BLENotify);
BLEUnsignedIntCharacteristic OxygenLevel ("2102", BLERead | BLENotify);

void setup()
{

  Serial.begin(115200);
//  Serial.begin(9600);
  while(!Serial); //We must wait for Teensy to come online

  if (!BLE.begin()) {
    Serial.println("BLE failed to Initiate");
    delay(500);
    while (1);
  }
  
  pinMode(smokeA0, INPUT);
  
  BLE.setLocalName("Pulse Beat oxy Sensor");
  BLE.setAdvertisedService(customService);
  
  customService.addCharacteristic(HeartRate);
  customService.addCharacteristic(OxygenLevel);
 
  BLE.addService(customService);
  HeartRate.writeValue(1);
  OxygenLevel.writeValue(1);
    
  BLE.advertise();
  
  //Serial.println("Bluetooth device is now active, waiting for connections...");

  digitalWrite(LED_BUILTIN, LOW);

  oled.begin();
  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setTextColor(1);
  oled.setCursor(0, 0);
   
  oled.println("Initializing pulse oximeter..");
  oled.display();
  //Serial.print("Initializing pulse oximeter..");

  byte ledBrightness = 70; //Options: 0=Off to 255=50mA
  byte sampleAverage = 1; //Options: 1, 2, 4, 8, 16, 32
  byte ledMode = 2; //Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
  int sampleRate = 400; //Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
  int pulseWidth = 69; //Options: 69, 118, 215, 411
  int adcRange = 16384; //Options: 2048, 4096, 8192, 16384


  if (particleSensor.begin(Wire, I2C_SPEED_FAST) == false) //Use default I2C port, 400kHz speed
  {
    //Serial.println("FAILED");
    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setTextColor(1);
    oled.setCursor(0, 0);
    oled.println("FAILED");
    oled.display();
    while (true)
    {
      delay(10000);
    }
    
  }
  else
  {
    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setTextColor(1);
    oled.setCursor(0, 0);
    oled.println("SUCCESS");
    oled.display();
    //Serial.println("SUCCESS");
  }
  //pox.setOnBeatDetectedCallback(onBeatDetected);
    particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange); //Configure sensor with these settings

}

void loop()
{
  
   //pox.update();
   
  //if (millis() - tsLastReport > REPORTING_PERIOD_MS) {

  particleSensor.check(); //Check the sensor

  while (particleSensor.available()) {
       // read stored IR
      //Serial.print(micros());
      //Serial.print(",");
      Serial.print(particleSensor.getFIFOIR());
      Serial.print(",");
      // read stored red
      Serial.println(particleSensor.getFIFORed());
      // read next set of samples
      particleSensor.nextSample();

      count++;

      if (count > 500)
      {
        int sensorValue = analogRead(smokeA0); // read analog input pin 0

        oled.clearDisplay();
        oled.setTextSize(2);
        oled.setTextColor(1);
        oled.setCursor(0,16);
        oled.print( "AQI: ");
        oled.print(sensorValue);
        oled.println(" ppm");
        oled.display();
        count = 0;
      }
  /*
    Serial.print("Heart BPM:");
    Serial.print(pox.getHeartRate());
    Serial.print("-----");
    Serial.print("Oxygen Percent:");
    Serial.print(pox.getSpO2());
    Serial.println("\n");
    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setTextColor(1);
    oled.setCursor(0,16);
    oled.println(pox.getHeartRate());
     
    oled.setTextSize(1);
    oled.setTextColor(1);
    oled.setCursor(0, 0);
    oled.println("Heart BPM");
     
    oled.setTextSize(1);
    oled.setTextColor(1);
    oled.setCursor(0, 30);
    oled.println("Spo2");
     
    oled.setTextSize(1);
    oled.setTextColor(1);
    oled.setCursor(0,45);
    oled.println(pox.getSpO2());
    oled.display();
    */
    tsLastReport = millis();
    //delay(100);
  }
 
  //Serial.println("Out of loop");

  BLEDevice central = BLE.central();
  if (central)
  {
    Serial.print("Connected to central: ");
    Serial.println(central.address());
    digitalWrite(LED_BUILTIN, HIGH);
    while (central.connected())
    {
      /*
        HeartRate.writeValue(pox.getHeartRate());
        OxygenLevel.writeValue(pox.getSpO2());
        */
    }
  }
  else
  {
      digitalWrite(LED_BUILTIN, LOW); 
  }
    //delay(connectDelay);
}
 
