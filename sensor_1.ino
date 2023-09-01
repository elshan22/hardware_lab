#include <WiFi.h>
#include <PubSubClient.h>
#include <random>

const char* ssid = "raspinetwork";
const char* password = "raspi427";

const char* mqtt_server = "192.168.154.176";
const int mqtt_port = 1883;

int timer = 1;

int last_time = 0;
double last_temp[12];
double sigma = 3;
double mean = 25;

std::default_random_engine generator;
std::normal_distribution<float> distribution(mean, sigma);

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
    if (client.connect("sensor#1")) {
      Serial.println("connected");
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

double get_mean() {
  double mean = 0;
  for (int i = 0 ; i < 12; i++) mean += last_temp[i];
  return mean / 12;
}

double get_std(double mean) {
  double sigma = 0;
  for (int i = 0 ; i < 12; i++) sigma += (last_temp[i] - mean)*(last_temp[i] - mean);
  return sqrt(sigma / 12);
}

void setup() {
  pinMode(ledPin, OUTPUT);
  Serial.begin(115200);
  
  ledcSetup(channel, freq, resolution);
  ledcAttachPin(ledPin, channel);

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);

  for (int i = 0 ; i < 12; i++) 
    last_temp[i] = distribution(generator);

  mean = get_mean();
  sigma = get_std(mean);

}

void loop() {
  if (!client.connected()) {
    connect_mqttServer();
  }
  client.loop();
  if (millis() - last_time > 5000) {
    last_time = millis();
    double new_data = distribution(generator);
    for (int i = 11; i > 0; i--) {
      last_temp[i] = last_temp[i-1];
    }
    last_temp[0] = new_data;
    mean = get_mean();
    sigma = get_std(mean);
    client.publish("raspi/sensors", String("{\"room\": 1, \"node_id\": \"" + WiFi.macAddress() + "\", \"current_temperature\": " + String(new_data) + ", \"temperature_mean\": " + String(mean) + ", \"recent_changes\": " + String(sigma) + "}").c_str());
  }
}
