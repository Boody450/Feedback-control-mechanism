.
#include <Servo.h>
#include <HX711_ADC.h>
#if defined(ESP8266)|| defined(ESP32) || defined(AVR)
#include <EEPROM.h>
#endif
unsigned long myTime;
unsigned long lastTime = 6000;
float lastWeight = 0;
//pins:
Servo servo;
const int HX711_dout = 8; 
const int HX711_sck = 9; 
//HX711 constructor:
HX711_ADC LoadCell(HX711_dout, HX711_sck);
int volume ; 
int count = 0;
int count1 = 0;
float density ;
const int calVal_eepromAdress = 0;
unsigned long t = 0;
static boolean newDataReady = 0;
const int serialPrintInterval = 0;
void setup() {
  Serial.begin(57600); delay(10);
  Serial.println();
  Serial.println("Starting...");


  LoadCell.begin();
  float calibrationValue; 
  calibrationValue = 624.38; 
  
#if defined(ESP8266)|| defined(ESP32)
#endif
  EEPROM.get(calVal_eepromAdress, calibrationValue); 

  unsigned long stabilizingtime = 2000;
  boolean _tare = true; 
  LoadCell.start(stabilizingtime, _tare);
  if (LoadCell.getTareTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
    while (1);
  }
  else {
    LoadCell.setCalFactor(calibrationValue); // set calibration value (float)
    Serial.println("Startup is complete");
  }
  ////////////////////////////////////////////////////////////////////////////////////////////
  //increase value to slow down serial print activity
  volume = 63;
  servo.attach(10);
  servo.write(90);
  delay(1000);
}

void loop() {
  myTime = millis();

  if (LoadCell.update()) newDataReady = true;
  
  if (newDataReady) {
    if (millis() > t + serialPrintInterval) {
      float i = LoadCell.getData();
      Serial.println("Load_cell output val: " + String(i));
      density = i/volume ;
        Serial.println("Density: "+String(density));
      //Serial.println(i);
      if (myTime > lastTime)
      {
        lastTime = myTime + 6000;
        
        if (abs(i - lastWeight) < 1)
        {
          if (i < 1){
            servo.write(90);
          }
          }
          while ((i >= 60 && i <= 80) && (density >=1.1 && density <=1.3) && count < 4) {
            delay(2000);
            servo.write(180);
            delay(1000);
            servo.write(90);
            Serial.println("The block is ready for commercial use!");
           count++;
          }
          while ((i>1 && i <= 60) || i >= 80 || (density <=1.1 && i > 1) || density >=1.3 && count1 < 4) {
            delay(2000);
            servo.write(0);
            delay (1000);
            servo.write(90);
            count++;
            Serial.println("There was a problem reagarding the construction of the block!");
            if (i>1 && i <= 60){
            Serial.println("Mass is low!");}
              else if (i >= 80){
              Serial.println("Mass is high!");}
              else if (density <=1.1 && i>1)
              { Serial.println("Density is low!");}
              else if (density >=1.3){
              Serial.println("Density is high!");
              }
          } 
        
      }
    }
      newDataReady = 0;
      t = millis();


    }
  }



