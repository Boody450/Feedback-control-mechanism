/*
   -------------------------------------------------------------------------------------
   HX711_ADC
   Arduino library for HX711 24-Bit Analog-to-Digital Converter for Weight Scales


/*
   Settling time (number of samples) and data filtering can be adjusted in the config.h file
   For calibration and storing the calibration value in eeprom, see example file "Calibration.ino"

   The update() function checks for new data and starts the next conversion. In order to acheive maximum effective
   sample rate, update() should be called at least as often as the HX711 sample rate; >10Hz@10SPS, >80Hz@80SPS.
   If you have other time consuming code running (i.e. a graphical LCD), consider calling update() from an interrupt routine,
   see example file "Read_1x_load_cell_interrupt_driven.ino".

*/
#include <Servo.h>
#include <HX711_ADC.h>
#if defined(ESP8266)|| defined(ESP32) || defined(AVR)
#include <EEPROM.h>
#endif
unsigned long myTime;
unsigned long lastTime = 2000;
float lastWeight = 0;
//pins:
Servo servo;
const int HX711_dout = 13; //mcu > HX711 dout pin
const int HX711_sck = 12; //mcu > HX711 sck pin
//HX711 constructor:
HX711_ADC LoadCell(HX711_dout, HX711_sck);

const int calVal_eepromAdress = 0;
unsigned long t = 0;
static boolean newDataReady = 0;
const int serialPrintInterval = 0;
void setup() {
  Serial.begin(57600); delay(10);
  Serial.println();
  Serial.println("Starting...");


  LoadCell.begin();
  //LoadCell.setReverseOutput(); //uncomment to turn a negative output value to positive
  float calibrationValue; // calibration value (see example file "Calibration.ino")
  calibrationValue = 696.0; // uncomment this if you want to set the calibration value in the sketch
#if defined(ESP8266)|| defined(ESP32)
  //EEPROM.begin(512); // uncomment this if you use ESP8266/ESP32 and want to fetch the calibration value from eeprom
#endif
  EEPROM.get(calVal_eepromAdress, calibrationValue); // uncomment this if you want to fetch the calibration value from eeprom

  unsigned long stabilizingtime = 2000; // preciscion right after power-up can be improved by adding a few seconds of stabilizing time
  boolean _tare = true; //set this to false if you don't want tare to be performed in the next step
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

  servo.attach(8);
  servo.write(90);
  delay(1000);
}

void loop() {
  myTime = millis();

  // check for new data/start next conversion:
  if (LoadCell.update()) newDataReady = true;

  // get smoothed value from the dataset:
  if (newDataReady) {
    if (millis() > t + serialPrintInterval) {
      float i = LoadCell.getData();
      Serial.print("Load_cell output val: ");
      Serial.println(i);
      if (myTime > lastTime)
      {
        lastTime = myTime + 1000;
        if (abs(i - lastWeight) < 10)
        {
          if (i < 1) {
            servo.write(90);
          }
          else if (i <= 50 || i >= 100) {
            servo.write(0);
          } else if (i >= 50 || i <= 100) {
            servo.write(180);
          }
        }
        lastWeight = i;
      }

      newDataReady = 0;
      t = millis();
    

    }
  }
}

// receive command from serial terminal, send 't' to initiate tare operation:
