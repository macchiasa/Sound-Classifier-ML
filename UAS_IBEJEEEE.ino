#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 16, 2);
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

const char* ssid = "your_wifi_ssid";
const char* password = "your_wifi_password";
const char* mqttBroker = "test.mosquitto.org";
const int mqttPort = 1883;
const char* mqttTopic = "nodered-elka";
const char* mqttResultTopic = "nodered-elka-results";

int num_Measure = 20;
int pinSignal = A0;
int PIN_QUIET = 3;
int PIN_MODERATE = 4;
int PIN_LOUD = 5;
long Sound_signal;
long sum = 0;
long level = 0;
int soundlow = 40;
int soundmedium = 150;

void setup() {
  pinMode(pinSignal, INPUT);
  pinMode(PIN_QUIET, OUTPUT);
  pinMode(PIN_MODERATE, OUTPUT);
  pinMode(PIN_LOUD, OUTPUT);

  digitalWrite(PIN_QUIET, LOW);
  digitalWrite(PIN_MODERATE, LOW);
  digitalWrite(PIN_LOUD, LOW);

  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.backlight();
  lcd.clear();

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Set MQTT server and callback function
  mqttClient.setServer(mqttBroker, mqttPort);
  mqttClient.setCallback(callback);

  // Connect to MQTT broker
  while (!mqttClient.connected()) {
    if (mqttClient.connect("ESP8266Client")) {
      Serial.println("Connected to MQTT broker");
      mqttClient.subscribe(mqttTopic);
    } else {
      delay(1000);
      Serial.println("Connecting to MQTT broker...");
    }
  }
}

void loop() {
  mqttClient.loop();

  for (int i = 0; i < num_Measure; i++) {
    Sound_signal = analogRead(pinSignal);
    sum = sum + Sound_signal;
  }
  level = sum / num_Measure;

  Serial.println(level);

  if (level < soundlow) {
    lcd.setCursor(0, 1);
    lcd.print("Level: Quiet");
    Serial.print(level);
    digitalWrite(PIN_QUIET, HIGH);
    digitalWrite(PIN_MODERATE, LOW);
    digitalWrite(PIN_LOUD, LOW);
  } else if (level > soundlow && level < soundmedium) {
    lcd.setCursor(0, 1);
    lcd.print("Level: Moderate");
    Serial.print(level);
    digitalWrite(PIN_QUIET, LOW);
    digitalWrite(PIN_MODERATE, HIGH);
    digitalWrite(PIN_LOUD, LOW);
  } else if (level > soundmedium) {
    lcd.setCursor(0, 1);
    lcd.print("Level: High");
    Serial.print(level);
    digitalWrite(PIN_QUIET, LOW);
    digitalWrite(PIN_MODERATE, LOW);
    digitalWrite(PIN_LOUD, HIGH);
  }
  sum = 0;
  delay(200);
  lcd.clear();
  
  // Publish the sound level classification result to MQTT broker
  String result = "";
  if (level < soundlow) {
    result = "Quiet";
  } else if (level > soundlow && level < soundmedium) {
    result = "Moderate";
  } else if (level > soundmedium) {
    result = "High";
  }
  mqttClient.publish(mqttResultTopic, result.c_str());
}

void callback(char* topic, byte* payload, unsigned int length) {
  // Handle incoming MQTT messages if needed
  // You can add your custom logic here for processing incoming messages
}
