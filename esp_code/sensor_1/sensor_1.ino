#include <WiFi.h>
#include <PubSubClient.h>
#include <random>

const char* ssid = "iksim";
const char* password = "ik3869662";

const char* mqtt_server = "192.168.154.176";

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
      delayMicroseconds(250);
  }
    delay(duration);
    for(int dutyCycle = 255; dutyCycle >= 0; dutyCycle--){
      ledcWrite(channel, dutyCycle);   
      delayMicroseconds(250);
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
    if (client.connect("node2_sensor")) {
      Serial.println("connected");
      client.subscribe("raspi/sensors");
      timer = 1;
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" trying again in " + String(timer) + " seconds");
      blink_led(2, 250);
      delay(timer * 1000);
      timer *= 2;
    }
  }
}

void setup() {
  pinMode(ledPin, OUTPUT);
  Serial.begin(115200);
  
  ledcSetup(channel, freq, resolution);
  ledcAttachPin(ledPin, channel);

  setup_wifi();
  client.setServer(mqtt_server, 8000);

  double real_mean = 0;
  double real_std = 0;
  for (int i = 0 ; i < 12; i++) {
    last_temp[i] = distribution(generator);
    real_mean += last_temp[i] / 12;
  }

  for (int i = 0 ; i < 12; i++)
    real_std += pow(last_temp[i] - mean, 2) / 12;

  real_std = sqrt(real_std);

  mean = real_mean;
  sigma = real_std;

}

void loop() {
  if (!client.connected()) {
    connect_mqttServer();
  }
  client.loop();
  if (millis() - last_time > 5000) {
    last_time = millis();
    double new_data = distribution(generator);
    mean += (new_data - last_temp[11]) / 12;
    sigma = sqrt(pow(sigma, 2) + (pow(new_data - mean, 2) - pow(last_temp[11] - mean, 2)) / 12);
    for (int i = 11; i > 0; i--) {
      last_temp[i] = last_temp[i-1];
    }
    last_temp[0] = new_data;
    client.publish("raspi/sensors", String("{\"room\": 1, \"node_id\": 2, \"current_temperature\": " + String(new_data) + ", \"temperature_mean\": " + String(mean) + ", \"recent_changes\": " + String(sigma) + "}").c_str());
  }
}
