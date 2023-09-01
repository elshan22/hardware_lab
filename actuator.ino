#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

StaticJsonDocument<1000> dict;

const char* ssid = "raspinetwork";
const char* password = "raspi427";

const char* mqtt_server = "192.168.154.176";
int mqtt_port = 1883;

int timer = 1;

WiFiClient espClient;
PubSubClient client(espClient);

#define ledPin 2
#define freq 5000
#define channel 0
#define resolution 8

void blink_led(unsigned int times, unsigned int duration){
  for (int t = 0; t < times; t++) {
    for(int dutyCycle = 0; dutyCycle <= 255; dutyCycle++){
      ledcWrite(channel, dutyCycle);
      delayMicroseconds(100);
  }
    delay(duration);
    for(int dutyCycle = 255; dutyCycle >= 0; dutyCycle--){
      ledcWrite(channel, dutyCycle);   
      delayMicroseconds(100);
  }
    delay(100);
  }
}

void setup_wifi() {
  delay(50);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.print(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    blink_led(1, 250);
    delay(1000);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void connect_mqttServer() {
  while (!client.connected()) {
    if (WiFi.status() != WL_CONNECTED){
      setup_wifi();
    }
    Serial.print("Attempting MQTT connection...");
    if (client.connect("actuator")) {
      Serial.println("connected");
      client.subscribe("raspi/command");
      client.publish("raspi/actuators", String("{\"mac_address\": \"" + WiFi.macAddress() + "\", \"room\": 1}").c_str());
      timer = 1;
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" trying again in " + String(timer) + " seconds");
      blink_led(2, 250);
      delay(timer * 1000);
      timer = std::min(2*timer, 60);
    }
  }
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  for (int i = 0; i < length; i++) {
    Serial.print((char) message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();
  deserializeJson(dict, messageTemp);
  if (dict["node_id"] == WiFi.macAddress()) {
    int blink_length = dict["blink_length"];
    int blink_count = dict["blink_count"];
    blink_led(blink_count, blink_length);
  }
}

void setup() {
  pinMode(ledPin, OUTPUT);
  Serial.begin(115200);
  
  ledcSetup(channel, freq, resolution);
  ledcAttachPin(ledPin, channel);

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    connect_mqttServer();
  }
  client.loop();
}
