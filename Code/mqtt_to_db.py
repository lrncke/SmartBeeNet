import paho.mqtt.client as mqtt
import mysql.connector
import json

# MQTT-Broker-Informationen
MQTT_BROKER = "localhost"  # Wenn der Broker auf dem Raspberry Pi läuft
MQTT_PORT = 1883  # Standard MQTT-Port
MQTT_TOPIC = "bienen/stock/#"  # Alle Bienenstock-Daten hören (mit Wildcard)
MQTT_KEEP_ALIVE_INTERVAL = 60

# MariaDB-Datenbank-Verbindung
db_connection = mysql.connector.connect(
    host="localhost",
    user="bienenuser",
    password="admin",  # Dein MariaDB-Passwort hier eintragen, wenn nötig
    database="bienenprojekt"
)

cursor = db_connection.cursor()

# MQTT Callback: Wenn eine Nachricht empfangen wird
def on_message(client, userdata, msg):
    print(f"Nachricht erhalten: {msg.topic} -> {msg.payload.decode()}")
    
    try:
        # Annahme, dass die Nachricht im JSON-Format gesendet wird
        data = json.loads(msg.payload.decode())
        stock_id = data.get("stock_id")
        temperature = data.get("temperature")
        weight = data.get("weight")
        
        if stock_id and temperature is not None and weight is not None:
            # SQL-Insert-Anfrage
            insert_query = """
                INSERT INTO bienenprojekt (bienenstock_id, temperatur, gewicht, zeitstempel)
                VALUES (%s, %s, %s, NOW())
            """
            cursor.execute(insert_query, (stock_id, temperature, weight))
            db_connection.commit()
            print(f"Neue Daten gespeichert: Stock {stock_id}, Temperatur {temperature}, Gewicht {weight}")
        else:
            print("Ungültige Daten erhalten!")
    except Exception as e:
        print(f"Fehler beim Verarbeiten der Nachricht: {e}")

# MQTT Setup
def setup_mqtt():
    client = mqtt.Client()

    # Callback für empfangene Nachrichten setzen
    client.on_message = on_message

    # Mit Broker verbinden
    client.connect(MQTT_BROKER, MQTT_PORT, MQTT_KEEP_ALIVE_INTERVAL)

    # Abonniere das Topic
    client.subscribe(MQTT_TOPIC)

    # Endlosschleife starten, um Nachrichten zu empfangen
    print("Starte den MQTT-Client und höre auf Nachrichten...")
    client.loop_forever()

if __name__ == "__main__":
    setup_mqtt()