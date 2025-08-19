#include <HX711_ADC.h>

// --- HX711 Pins ---
const int Cell1_dout = 22;
const int Cell1_sck  = 23;

const int Cell2_dout = 16;
const int Cell2_sck  = 17;

const int Cell3_dout = 4;
const int Cell3_sck  = 5;

const int Cell4_dout = 18;
const int Cell4_sck  = 19;

// --- HX711 Instanzen ---
HX711_ADC LoadCell1(Cell1_dout, Cell1_sck);
HX711_ADC LoadCell2(Cell2_dout, Cell2_sck);
HX711_ADC LoadCell3(Cell3_dout, Cell3_sck);
HX711_ADC LoadCell4(Cell4_dout, Cell4_sck);

// --- Gewichtsvariablen ---
float weight1 = NAN;
float weight2 = NAN;
float weight3 = NAN;
float weight4 = NAN;

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("Starte HX711 Multi-Wägezellen-Test mit startMultiple()...");

  // --- Initialisierung ---
  LoadCell1.begin();
  LoadCell2.begin();
  LoadCell3.begin();
  LoadCell4.begin();

  bool ok1 = LoadCell1.startMultiple(2000, false);
  bool ok2 = LoadCell2.startMultiple(2000, false);
  bool ok3 = LoadCell3.startMultiple(2000, false);
  bool ok4 = LoadCell4.startMultiple(2000, false);

  if (!ok1) Serial.println("❌ LoadCell1 NICHT bereit!");
  if (!ok2) Serial.println("❌ LoadCell2 NICHT bereit!");
  if (!ok3) Serial.println("❌ LoadCell3 NICHT bereit!");
  if (!ok4) Serial.println("❌ LoadCell4 NICHT bereit!");

  // --- Kalibrierfaktor grob setzen ---
  LoadCell1.setCalFactor(224.92);  // deinen Kalibrierwert einsetzen
  LoadCell2.setCalFactor(103.01);  // deinen Kalibrierwert einsetzen
  LoadCell3.setCalFactor(-95.95);  // deinen Kalibrierwert einsetzen
  LoadCell4.setCalFactor(105.65);

  Serial.println("→ Tare (Nullung) läuft...");
  LoadCell1.tare();
  LoadCell2.tare();
  LoadCell3.tare();
  LoadCell4.tare();
  Serial.println("✅ Tare abgeschlossen.");
}

void loop() {
  // Alle Zellen einzeln updaten
  if (LoadCell1.update()) {
    weight1 = LoadCell1.getData();
  }
  if (LoadCell2.update()) {
    weight2 = LoadCell2.getData();
  }
  if (LoadCell3.update()) {
    weight3 = LoadCell3.getData();
  }
  if (LoadCell4.update()) {
    weight4 = LoadCell4.getData();
  }

  // Alle 1000 ms ausgeben
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 1000) {
    Serial.println("----- Gewichtsausgabe -----");
    Serial.print("Zelle1: "); Serial.println(weight1, 2);
    Serial.print("Zelle2: "); Serial.println(weight2, 2);
    Serial.print("Zelle3: "); Serial.println(weight3, 2);
    Serial.print("Zelle4: "); Serial.println(weight4, 2);
    Serial.println("---------------------------");
    lastPrint = millis();
  }

  delay(2); // HX711 braucht etwas Ruhe
}
