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
RTC_DATA_ATTR long savedOffset = 0;

// --- HX711 Setup ---
const int HX711_dout = 22; // HX711 Daten Pin
const int HX711_sck = 23;  // HX711 Clock Pin
HX711_ADC LoadCell(HX711_dout, HX711_sck);

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
    Serial.println("✅ Deep Sleep Wake");
  } else {
    Serial.println("❌ Normaler Start");
  }

  // Sensoren initialisieren
  dht.begin();
  LoadCell.begin();
  LoadCell.start(2000, false); // nur stabilisieren, kein Tare
  LoadCell.setCalFactor(224.92);  // deinen Kalibrierwert einsetzen

  if (wakeReason != ESP_SLEEP_WAKEUP_TIMER) {
    Serial.println("→ Tare (nur bei Kaltstart)");
    LoadCell.tare(); // nullen
    savedOffset = LoadCell.getTareOffset();
    Serial.print("Offset gespeichert: ");
    Serial.println(savedOffset);
  } else {
    LoadCell.setTareOffset(savedOffset);
    Serial.print("Offset wiederhergestellt: ");
    Serial.println(savedOffset);
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
  float weight = -999;
  for (int i = 0; i < 40; ++i) {
    if (LoadCell.update()) {
      weight = LoadCell.getData();
      break;
    }
    delay(2);
  }
  Serial.print("Gewicht: ");
  Serial.println(weight, 2);

  // JSON String bauen
  String payload = "{";
  payload += "\"stock_id\":" + String(stock_id) + ",";
  payload += "\"temperature\":" + String(temperature, 2) + ",";
  payload += "\"weight\":" + String(weight, 2);
  payload += "}";

  Serial.print("Sende MQTT Payload: ");
  Serial.println(payload);

  // MQTT Nachricht senden
  if (!client.connected()) {
    connectMQTT();
  }
  client.publish(mqttTopic, payload.c_str());

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