#include <HX711_ADC.h>
#if defined(ESP8266) || defined(ESP32) || defined(AVR)
#include <EEPROM.h>
#endif

// Pins für jeden HX711
HX711_ADC LoadCell1(22, 23);
HX711_ADC LoadCell2(18, 19);
HX711_ADC LoadCell3(4, 5);
HX711_ADC LoadCell4(16, 17);

// Kalibrierungswerte (je HX711 individuell)
float calibrationValue1 = 224.92;
float calibrationValue2 = 225.10;
float calibrationValue3 = 223.80;
float calibrationValue4 = 224.50;

const float offsetWeight = 2.5; // kg oder Gramm, je nach Kalibrierung

unsigned long t = 0;

void setup() {
  Serial.begin(57600);
  delay(10);
  Serial.println();
  Serial.println("Starting...");

  // Start aller HX711
  LoadCell1.begin();
  LoadCell2.begin();
  LoadCell3.begin();
  LoadCell4.begin();

  // Kalibrierung setzen
  LoadCell1.setCalFactor(calibrationValue1);
  LoadCell2.setCalFactor(calibrationValue2);
  LoadCell3.setCalFactor(calibrationValue3);
  LoadCell4.setCalFactor(calibrationValue4);

  unsigned long stabilizingtime = 2000;
  boolean _tare = true;

  LoadCell1.start(stabilizingtime, _tare);
  LoadCell2.start(stabilizingtime, _tare);
  LoadCell3.start(stabilizingtime, _tare);
  LoadCell4.start(stabilizingtime, _tare);

  if (LoadCell1.getTareTimeoutFlag() || LoadCell2.getTareTimeoutFlag() ||
      LoadCell3.getTareTimeoutFlag() || LoadCell4.getTareTimeoutFlag()) {
    Serial.println("Timeout, check wiring!");
    while (1);
  }
  Serial.println("Startup complete.");
}

void loop() {
  static boolean newDataReady = 0;
  const int serialPrintInterval = 500; // Ausgabe-Intervall in ms

  // Update aller Sensoren
  bool newData = false;
  newData |= LoadCell1.update();
  newData |= LoadCell2.update();
  newData |= LoadCell3.update();
  newData |= LoadCell4.update();
  if (newData) newDataReady = true;

  // Gewichte auslesen, addieren, Offset abziehen
  if (newDataReady) {
    if (millis() > t + serialPrintInterval) {
      float weight1 = LoadCell1.getData();
      float weight2 = LoadCell2.getData();
      float weight3 = LoadCell3.getData();
      float weight4 = LoadCell4.getData();

      float totalWeight = weight1 + weight2 + weight3 + weight4 - offsetWeight;

      Serial.print("HX711 #1: ");
      Serial.print(weight1);
      Serial.print("\t HX711 #2: ");
      Serial.print(weight2);
      Serial.print("\t HX711 #3: ");
      Serial.print(weight3);
      Serial.print("\t HX711 #4: ");
      Serial.print(weight4);
      Serial.print("\t => Total (with offset -");
      Serial.print(offsetWeight);
      Serial.print("): ");
      Serial.println(totalWeight);

      newDataReady = 0;
      t = millis();
    }
  }

  // Tare per Terminalbefehl 't'
  if (Serial.available() > 0) {
    char inByte = Serial.read();
    if (inByte == 't') {
      LoadCell1.tareNoDelay();
      LoadCell2.tareNoDelay();
      LoadCell3.tareNoDelay();
      LoadCell4.tareNoDelay();
    }
  }

  if (LoadCell1.getTareStatus() || LoadCell2.getTareStatus() ||
      LoadCell3.getTareStatus() || LoadCell4.getTareStatus()) {
    Serial.println("Tare complete.");
  }
}
