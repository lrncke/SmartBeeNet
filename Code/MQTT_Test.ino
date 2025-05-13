// This example uses an ESP32 Development Board
// to connect to shiftr.io.
//
// You can check on your device after a successful
// connection here: https://www.shiftr.io/try.
//
// by Joël Gähwiler
// https://github.com/256dpi/arduino-mqtt

#include <WiFi.h>
#include <MQTT.h>

const char ssid[] = "MyBeeNet";
const char pass[] = "Me1nPasswort";

WiFiClient net;
MQTTClient client;

unsigned long lastMillis = 0;

void connect() {
  Serial.print("checking wifi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nWiFi connected!");
  Serial.println(WiFi.localIP());

  Serial.print("\nconnecting...");
  while (!client.connect("stock_1")) {
    Serial.print(".");
    delay(1000);
    Serial.print(" last error: ");
    Serial.println(client.lastError());
  }

  Serial.println("\nconnected!");

  // client.subscribe("/hello");
  // client.unsubscribe("/hello");
}

void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);

  // Note: Do not use the client in the callback to publish, subscribe or
  // unsubscribe as it may cause deadlocks when other things arrive while
  // sending and receiving acknowledgments. Instead, change a global variable,
  // or push to a queue and handle it in the loop after calling `client.loop()`.
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, pass);

  // Note: Local domain names (e.g. "Computer.local" on OSX) are not supported
  // by Arduino. You need to set the IP address directly.
  client.begin("192.168.1.1", net);
  // client.onMessage(messageReceived);

  connect();
}

void loop() {
  client.loop();
  delay(10);  // <- fixes some issues with WiFi stability

  float temperature = 25.5;
  float weight = 1.2;
  int stock_id = 1;

  // JSON-String manuell erstellen
  String payload = "{";
  payload += "\"stock_id\":" + String(stock_id) + ",";
  payload += "\"temperature\":" + String(temperature, 2) + ",";
  payload += "\"weight\":" + String(weight, 2);
  payload += "}";

  if (!client.connected()) {
    connect();
  }

  // publish a message roughly every second.
  if (millis() - lastMillis > 1000) {
    lastMillis = millis();
    client.publish("bienen/stock/1", payload.c_str());
  }
}
