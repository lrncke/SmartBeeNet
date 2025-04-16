#include <WiFi.h>
#include <PubSubClient.h>

// MQTT-Server (IP-Adresse oder URL deines Brokers)
const char* mqtt_server = "192.168.1.100";  // IP-Adresse deines Raspberry Pi

// WiFi-Zugangsdaten
const char* ssid = "DeinWiFi";  // Dein WLAN-SSID
const char* password = "DeinPasswort";  // Dein WLAN-Passwort

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  
  // Verbindung zum WiFi herstellen
  setup_wifi();
  
  // MQTT-Server verbinden
  client.setServer(mqtt_server, 1883);
  
  // In einer Schleife versuchen, mit dem Broker zu verbinden
  while (!client.connected()) {
    Serial.print("Versuche, mit dem MQTT-Server zu verbinden...");
    
    // Erstelle eine zufällige Client-ID
    String clientId = "ESP32Client" + String(random(0xffff), HEX);
    
    // Versuche, dich mit dem Broker zu verbinden
    if (client.connect(clientId.c_str())) {
      Serial.println("verbunden");
    } else {
      Serial.print("Fehler, rc=");
      Serial.print(client.state());
      Serial.println(" Versuche es in 5 Sekunden erneut.");
      delay(5000);
    }
  }
}

void loop() {
  // Wenn der Client verbunden ist, sende regelmäßig Daten
  if (client.connected()) {
    // Beispiel: Temperatursensor oder Gewichtssensor lesen (hier als Beispiel simuliert)
    float temperature = 25.5;  // Beispielwert
    float weight = 1.2;        // Beispielwert
    
    // Erstelle eine Nachricht als JSON oder einfach als Text
    String message = "Temp:" + String(temperature) + ",Weight:" + String(weight);
    
    // Sende die Daten an ein Topic (z. B. "beehive/data")
    client.publish("beehive/data", message.c_str());
    
    Serial.println("Daten gesendet: " + message);
    
    // Warten, bevor die nächsten Daten gesendet werden
    delay(10000);  // 10 Sekunden
  } else {
    // Wenn die Verbindung unterbrochen ist, versuche sie erneut herzustellen
    reconnect();
  }
  
  // MQTT-Client-Verbindungen aufrechterhalten
  client.loop();
}

void setup_wifi() {
  // Verbindung zum WiFi herstellen
  Serial.print("Verbinde mit WLAN: ");
  Serial.println(ssid);
  
  while (WiFi.begin(ssid, password) != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  
  Serial.println("WLAN verbunden");
}

void reconnect() {
  // Versuche, die Verbindung wiederherzustellen
  while (!client.connected()) {
    Serial.print("Versuche, die Verbindung zum MQTT-Server wiederherzustellen...");
    
    String clientId = "ESP32Client" + String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str())) {
      Serial.println("verbunden");
    } else {
      Serial.print("Fehler, rc=");
      Serial.print(client.state());
      Serial.println(" Versuche es in 5 Sekunden erneut.");
      delay(5000);
    }
  }
}
