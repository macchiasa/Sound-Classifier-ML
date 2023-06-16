#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <MovingAverage.h>

const char* ssid = "asa";
const char* password = "........";
const char* mqtt_server = "test.mosquitto.org";

WiFiClient espClient;
PubSubClient client(espClient);

const int sampleWindow = 50;
unsigned int sample;

unsigned long now = millis();

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  client.setServer(mqtt_server, 1883);

  while (!client.connected()) {
    if (client.connect("ESP8266Client")) {
      Serial.println("Connected to MQTT broker");
    } else {
      Serial.print("Failed to connect to MQTT broker, rc=");
      Serial.print(client.state());
      delay(2000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);
  } else {
    digitalWrite(BUILTIN_LED, HIGH);
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.publish("nodered-elka", "Dah tersambung nih...");    // Send "terkoneksi" message to "nodered-elka" topic
      client.subscribe("receiver-elka");                          // Subscribe to "sender-elka" topic
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(9600);
  setup_wifi();
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long currentMillis = millis();
  if (currentMillis - now >= 500) {
    now = currentMillis;

    // Measure sound and send data to Node-RED
    unsigned long startMillis = millis();
    float peakToPeak = 0;
    unsigned int signalMax = 0;
    unsigned int signalMin = 1024;

    while (millis() - startMillis < sampleWindow) {
      sample = analogRead(A0);
      if (sample < 1024) {
        if (sample > signalMax) {
          signalMax = sample;
        } else if (sample < signalMin) {
          signalMin = sample;
        }
      }
    }

    peakToPeak = signalMax - signalMin;
    int db = map(peakToPeak, 20, 900, 49.5, 90);
    Serial.print("dB: ");
    Serial.println(db);

    String strTemp = String(db);
    client.publish("nodered-elka", strTemp.c_str());
  }
}
