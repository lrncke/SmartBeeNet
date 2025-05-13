#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "MyBeeNet";
const char* password = "Me1nPasswort";
const char* mqtt_server = "192.168.1.1";

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  delay(100);  // kurze Stabilisierung
  
  setup_wifi();
  client.setServer(mqtt_server, 1883);

  if (client.connect("ESP32Client")) {
    float temperature = 25.5;
    float weight = 1.2;
    String message = "Temp:" + String(temperature) + ",Weight:" + String(weight);
    client.publish("beehive/data", message.c_str());
    Serial.println("Daten gesendet: " + message);
  }

  Serial.println("Gehe in Deep Sleep...");
  esp_sleep_enable_timer_wakeup(10 * 60 * 1000000ULL);  // 10 Minuten
  esp_deep_sleep_start();
}

void loop() {
  // wird nach Deep Sleep nicht erreicht
}

void setup_wifi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Verbunden mit WLAN");
}
