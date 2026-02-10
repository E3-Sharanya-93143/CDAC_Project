/*void setup() {
  Serial.begin(115200);   // UART from STM32
}

void loop() {
  if (Serial.available()) {
    String data = Serial.readStringUntil('\n');
    Serial.println("Received: " + data);
  }
}*/
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

/* ---------- WIFI ---------- */
const char* ssid = "UnitedPG202";
const char* password = "United202";

/* ---------- THINGSPEAK MQTT ---------- */
const char* mqttServer = "mqtt3.thingspeak.com";
const int mqttPort = 1883;

const char* mqttUsername = "3240173";          // CHANNEL ID
const char* mqttPassword = "UICUKHX86UTWK1CG"; // WRITE API KEY

/* ---------- OBJECTS ---------- */
WiFiClient espClient;
PubSubClient client(espClient);

/* ---------- WIFI CONNECT ---------- */
void connectWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected");
}

/* ---------- MQTT CONNECT ---------- */
void connectMQTT() {
  while (!client.connected()) {
    Serial.print("Connecting MQTT...");
    if (client.connect("STM32_NodeMCU", mqttUsername, mqttPassword)) {
      Serial.println("Connected");
    } else {
      Serial.print("Failed, rc=");
      Serial.println(client.state());
      delay(3000);
    }
  }
}

/* ---------- SETUP ---------- */
void setup() {
  Serial.begin(115200);   // STM32 → ESP UART
  connectWiFi();
  client.setServer(mqttServer, mqttPort);
}

/* ---------- LOOP ---------- */
void loop() {

  if (!client.connected()) {
    connectMQTT();
  }
  client.loop();

  if (Serial.available()) {
    String data = Serial.readStringUntil('\n');
    data.trim();

    if (data.length() > 0) {
      String payload = "field1=" + data;

      Serial.print("Publishing: ");
      Serial.println(payload);

      client.publish(
        "channels/3240173/publish",
        payload.c_str()
      );
    }
  }
}
