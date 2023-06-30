#include <Arduino.h>
#include <freertos/FreeRTOS.h>

#include <PubSubClient.h>
#include <WiFi.h>

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <BH1750.h>
#include <unordered_map>

const char* mqttServer = "192.168.43.175";
const int mqttPort = 1883;
const char* clientID = "ESP32id";
const char *topicT = "Temperature", *topicP="Pressure", *topicA="Altitude", *topicL="Luminosity";

// WiFi
const char* ssid = "TeamBerrySpot";                
const char* wifi_password = "98765432";
WiFiClient wifiClient;
PubSubClient client(mqttServer,mqttPort,wifiClient);

// Sensori
#define I2C_SDA 33
#define I2C_SCL 32
const float seaLevelhPa = 1013.25;
Adafruit_BMP280 bmp;  // Oggetto per il sensore BMP280
BH1750 lightMeter;    // Oggetto per il sensore BH1750
TwoWire I2C = TwoWire(0);

/*
PRIORITY  TASK
  1       clock
  2       luminosuty
  3       pressione
  4       send message
*/

// Custom function to connect to the MQTT broker via WiFi
void connect_MQTT() {
  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected!");

  // Connessione al broker MQTT
  Serial.println("Connecting to MQTT broker...");
  if (client.connect(clientID)) {
    Serial.println("Connected to MQTT Broker!");
  }
  else {
    Serial.println("Connection to MQTT Broker failed...");
  }
}

std::unordered_map<std::string, float> readSensors() {
  std::unordered_map<std::string, float> values;

  // BMP280
  values["TEMPERATURE"] = bmp.readTemperature();
  values["PRESSURE"] = bmp.readPressure();
  values["ALTITUDE"] = bmp.readAltitude();

  // BH1750
  values["LUMINOSITY"] = lightMeter.readLightLevel();

  // MessageToSerial
  Serial.print("Temperature: ");  Serial.print(values["TEMPERATURE"]);  Serial.println(" °C");
  Serial.print("Pressure:    ");  Serial.print(values["PRESSURE"]);  Serial.println(" hPa");
  Serial.print("Altitude:    ");  Serial.print(values["ALTITUDE"]);  Serial.println(" m");
  Serial.print("Luminosity:  ");  Serial.print(values["LUMINOSITY"]);   Serial.println(" lx");

  return values;
}

void sendMessage(PubSubClient& client, const char* topic, String& message){
  if (client.publish(topic, message.c_str())) {
    Serial.println("TOPIC: "+(String)topic+" -> message sent");
  }
  else {
    Serial.println("TOPIC: "+(String)topic+" -> FAILED to send message");
  }
}

void setup() {
  // SERIALE
  Serial.begin(9600);
  Serial.println("(O) SERIALE CONNESSA");

  // WIFI & MQTT
  WiFi.begin(ssid, wifi_password);
  connect_MQTT();

  // SENSORI
  delay(4000);
  I2C.begin(I2C_SDA, I2C_SCL, 100000);
  bmp = Adafruit_BMP280(&I2C);
  bmp.begin(0x77); 
  lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, 0x23, &I2C);
}

void loop() {
  Serial.println("|===========BEGIN LOOP===========");
  
  // Check connection
  connect_MQTT();

  // Reading sensors
  std::unordered_map<std::string, float> values = readSensors();
  
  // Publishing values  
  sendMessage(client,topicT,"Temperature: "+String(values["TEMPERATURE"])+" °C");
  sendMessage(client,topicP,"Pressure: "+String(values["PRESSURE"])+" hPa");
  sendMessage(client,topicA,"Altitude: "+String(values["ALTITUDE"])+" m");
  sendMessage(client,topicL,"Luminosity: "+String(values["LUMINOSITY"])+" lx");

  Serial.println("|=========== END LOOP ===========");
}


