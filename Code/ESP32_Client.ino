#include <WiFi.h>
#include <MQTT.h>
#include "DHT.h"
#include <HX711_ADC.h>

int stock_id = 1;

// --- DHT Sensor Setup ---
#define DHTPIN 2          // Pin am ESP32, wo DHT11 angeschlossen ist
#define DHTTYPE DHT11     // Sensor Typ
DHT dht(DHTPIN, DHTTYPE);

// Speichert Tare-Offset im RTC-RAM (überlebt Deep Sleep!)
RTC_DATA_ATTR long savedOffset1 = 0;
RTC_DATA_ATTR long savedOffset2 = 0;
RTC_DATA_ATTR long savedOffset3 = 0;
RTC_DATA_ATTR long savedOffset4 = 0;

const int Cell1_dout = 22; //mcu > HX711 dout pin
const int Cell1_sck = 23; //mcu > HX711 sck pin

const int Cell2_dout = 16;
const int Cell2_sck = 17;

const int Cell3_dout = 4;
const int Cell3_sck = 5;

const int Cell4_dout = 18;
const int Cell4_sck = 19;

//=============================================
float weight1 = -999;
float weight2 = -999;
float weight3 = -999;
float weight4 = -999;
  //=============================================

//HX711 constructor:
HX711_ADC LoadCell(Cell1_dout, Cell1_sck);

HX711_ADC LoadCell2(Cell2_dout, Cell2_sck);
HX711_ADC LoadCell3(Cell3_dout, Cell3_sck);
HX711_ADC LoadCell4(Cell4_dout, Cell4_sck);

// --- WLAN & MQTT Setup ---
const char ssid[] = "MyBeeNet";
const char pass[] = "Me1nPasswort";

WiFiClient net;
MQTTClient client;

unsigned long lastMillis = 0;

// MQTT Broker IP (ersetze durch deinen Broker)
const char* mqttBroker = "192.168.1.1";
const char* mqttTopic = "bienen/stock/1";

// Funktion zur WLAN Verbindung
void connectWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nWiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

// Funktion zur MQTT Verbindung
void connectMQTT() {
  Serial.print("Connecting to MQTT broker...");
  while (!client.connect("esp32_client")) {
    Serial.print(".");
    delay(500);
    Serial.print(" Last error: ");
    Serial.println(client.lastError());
  }
  Serial.println("\nMQTT connected!");
}

void setup() {
  Serial.begin(115200);
  delay(200); // Warte auf seriellen Monitor

  esp_sleep_wakeup_cause_t wakeReason = esp_sleep_get_wakeup_cause();
  Serial.print("Wakeup-Code: ");
  Serial.println((int)wakeReason);

  if (wakeReason == ESP_SLEEP_WAKEUP_TIMER) {
    Serial.println("Deep Sleep Wake");
  } else {
    Serial.println("Normaler Start");
  }

  // Sensoren initialisieren
  dht.begin();

//=====================================
  LoadCell.begin();
  LoadCell2.begin();
  LoadCell3.begin();
  LoadCell4.begin();
//=====================================

//=====================================
  LoadCell.start(2000, false); // nur stabilisieren, kein Tare
  LoadCell2.start(2000, false);
  LoadCell3.start(2000, false);
  LoadCell4.start(2000, false);
//=====================================

//=====================================
  LoadCell.setCalFactor(224.92);  // deinen Kalibrierwert einsetzen
  LoadCell2.setCalFactor(103.01);  // deinen Kalibrierwert einsetzen
  LoadCell3.setCalFactor(-95.95);  // deinen Kalibrierwert einsetzen
  LoadCell4.setCalFactor(105.65);  // deinen Kalibrierwert einsetzen
//=====================================



  if (wakeReason != ESP_SLEEP_WAKEUP_TIMER) {
    Serial.println("→ Tare (nur bei Kaltstart)");

    //===========================================
    LoadCell.tare(); // nullen
    LoadCell2.tare();
    LoadCell3.tare();
    LoadCell4.tare();
    //===========================================

    //===========================================
    savedOffset1 = LoadCell.getTareOffset();
    savedOffset2 = LoadCell2.getTareOffset();
    savedOffset3 = LoadCell3.getTareOffset();
    savedOffset4 = LoadCell4.getTareOffset();
    //===========================================

    Serial.print("Offset gespeichert: ");

    //===========================================
    printOffsets();
    //===========================================


  } else {

    //===========================================
    LoadCell.setTareOffset(savedOffset1);
    LoadCell2.setTareOffset(savedOffset2);
    LoadCell3.setTareOffset(savedOffset3);
    LoadCell4.setTareOffset(savedOffset4);
    //===========================================


    Serial.print("Offset wiederhergestellt: ");
    printOffsets();
  }

  // WLAN und MQTT starten
  connectWiFi();
  client.begin(mqttBroker, net);
  connectMQTT();

  // Temperatur aus DHT11 auslesen (°C)
  float temperature = dht.readTemperature();
  if (isnan(temperature)) {
    Serial.println("Fehler beim Lesen der Temperatur vom DHT11");
    temperature = -999; // Fehlerwert
  } else {
    Serial.print("Temperatur: ");
    Serial.print(temperature);
    Serial.println(" °C");
  }

  // Gewicht messen
  


  for (int i = 0; i < 40; ++i) {
    if (LoadCell.update()) {
      weight1 = LoadCell.getData();
      weight2 = LoadCell2.getData();
      weight3 = LoadCell3.getData();
      weight4 = LoadCell4.getData();
      break;
    }
    delay(2);
  }
  
  printWeights();

  // JSON String bauen
  /*String payload = "{";
  payload += "\"stock_id\":" + String(stock_id) + ",";
  payload += "\"temperature\":" + String(temperature, 2) + ",";
  payload += "\"weight\":" + String(weight, 2);
  payload += "}";

  Serial.print("Sende MQTT Payload: ");
  Serial.println(payload);*/

  // MQTT Nachricht senden
  if (!client.connected()) {
    connectMQTT();
  }
  //client.publish(mqttTopic, payload.c_str());

  // MQTT Loop für kurze Zeit aufrufen um senden sicherzustellen
  client.loop();
  delay(500);

  // Deep Sleep für 5 Sekunden
  Serial.println("→ Gehe jetzt in Deep Sleep (5 s)");
  delay(1000);
  esp_sleep_enable_timer_wakeup(5 * 1000000ULL);
  esp_deep_sleep_start();
}

void loop() {
  // Wird nicht genutzt, da wir nach setup() in den Deep Sleep gehen
}

void printOffsets()
{
    Serial.print("Wert1:  ");Serial.println(savedOffset1);
    Serial.print("Wert2:  ");Serial.println(savedOffset2);
    Serial.print("Wert3:  ");Serial.println(savedOffset3);
    Serial.print("Wert4:  ");Serial.println(savedOffset4);
}

void printWeights()
{
  Serial.println("Aktualisierte Gewichte:");
  Serial.print("Zelle1:  "); Serial.print(weight1); Serial.print("Zelle2:  "); Serial.print(weight2); Serial.print("Zelle3:  "); Serial.print(weight3); Serial.print("Zelle4:  "); Serial.println(weight4);
}