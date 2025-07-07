#include <WiFi.h>
#include <MQTT.h>
#include "DHT.h"
#include <HX711_ADC.h>

int stock_id = 1;

// --- DHT Sensor Setup ---
#define DHTPIN 2          // Pin am ESP32, wo DHT11 angeschlossen ist
#define DHTTYPE DHT11     // Sensor Typ
DHT dht(DHTPIN, DHTTYPE);

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
  delay(1000); // Warte auf seriellen Monitor

  // Überprüfen, warum ESP32 aus Deep Sleep erwacht ist
  if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER) {
    Serial.println("ESP32 ist aus Deep Sleep erwacht.");
  } else {
    Serial.println("Normaler Start.");
  }

  // Sensoren initialisieren
  dht.begin();
  LoadCell.begin();
  LoadCell.start(2000, true); // 2 Sekunden Stabilisierung + Tare
  LoadCell.setCalFactor(696.0); // Kalibrierwert anpassen falls nötig

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

  // Gewicht aus HX711 auslesen
  LoadCell.update(); // Daten aktualisieren
  float weight = LoadCell.getData();
  Serial.print("Gewicht: ");
  Serial.println(weight);

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

  Serial.println("ESP32 geht jetzt in den Deep Sleep für 60 Sekunden...");

  // Deep Sleep konfigurieren: 60 Sekunden (60 * 1.000.000 Mikrosekunden)
  esp_sleep_enable_timer_wakeup(60 * 1000000);
  esp_deep_sleep_start();
}

void loop() {
  // Wird nicht genutzt, da wir nach setup() in den Deep Sleep gehen
}