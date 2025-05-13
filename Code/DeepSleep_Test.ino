void setup() {
  // Seriellen Monitor initialisieren
  Serial.begin(115200);
  delay(1000);  // Warte, um sicherzustellen, dass der serielle Monitor geöffnet ist

  // Überprüfen, ob der ESP32 aus dem Deep Sleep aufgewacht ist
  if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER) {
    Serial.println("Der ESP32 ist wieder aus dem Deep Sleep aufgewacht!");
    
    // Schleife von 0 bis 9
    for (int i = 0; i < 10; i++) {
      Serial.println(i);
      delay(500);  // Verzögerung, um die Zahlen gut sichtbar anzuzeigen
    }
  } else {
    Serial.println("ESP32 geht jetzt in den Deep Sleep...");
  }

  // Der ESP32 schläft für 60 Sekunden (1 Minute)
  esp_sleep_enable_timer_wakeup(60 * 1000000); // Zeit in Mikrosekunden

  // Deep Sleep Modus aktivieren
  esp_deep_sleep_start();
}

void loop() {
  // Der ESP32 wird nie hier ankommen, weil er in den Deep Sleep versetzt wird
}
