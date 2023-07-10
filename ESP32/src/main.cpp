#include <Arduino.h>
#include <freertos/FreeRTOS.h>

#include <PubSubClient.h>
#include <WiFi.h>

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <BH1750.h>
#include <RTClib.h>

// WiFi
const char *ssid = "TeamBerrySpot";
const char *wifi_password = "98765432";
WiFiClient wifiClient;

// MQTT
const char *mqttServer = "192.168.43.175";
const int mqttPort = 1883;
const char *clientID = "ESP32id";
const char *topicT = "Temperature", *topicP = "Pressure", *topicA = "Altitude", *topicL = "Luminosity";
PubSubClient client(mqttServer, mqttPort, wifiClient);

// Sensori
#define I2C_SDA 33
#define I2C_SCL 32
#define RTC_SDA 27
#define RTC_SCL 26
const float seaLevelhPa = 1013.25;
Adafruit_BMP280 bmp;
BH1750 lightMeter;
RTC_PCF8523 rtc;
TwoWire I2C = TwoWire(0), RTC = TwoWire(1);

// Shared Data
typedef struct
{
  char timestamp[22];
  char topic[12];
  float value;
} SingleData;
QueueHandle_t messageQueue;
const int QueueSize = 100;
SemaphoreHandle_t queueMUTEX, i2cMUTEX;
const TickType_t Frequency_T = pdMS_TO_TICKS(2000),
                 Frequency_P = pdMS_TO_TICKS(1000),
                 Frequency_L = pdMS_TO_TICKS( 500);

// Custom function to connect to the MQTT broker via WiFi
void connect_MQTT(){
  Serial.println("SETUP ) connecting to WiFi...");
  int k=0;
  while (WiFi.status() != WL_CONNECTED){
    if(k%5==0)
      WiFi.begin(ssid, wifi_password);
    k++;
    delay(1000);
    Serial.print(".");
  }
  Serial.println("SETUP ) WiFi connected.");

  // Connessione al broker MQTT
  Serial.println("SETUP ) connecting to MQTT broker.");
  if (client.connect(clientID)){
    Serial.println("SETUP ) connected to MQTT Broker.");
    client.subscribe("SetTime");
  }else{
    Serial.println("SETUP ) connection to MQTT Broker failed.");
  }
}

void TaskMESSAGE(void *pvParameters){
  SingleData toSend;
  BaseType_t result = pdFALSE;
  while (1){
    if(xSemaphoreTake(queueMUTEX, portMAX_DELAY))
      if (uxQueueSpacesAvailable(messageQueue) != QueueSize)
      result = xQueueReceive(messageQueue, &toSend, pdMS_TO_TICKS(10000));
    xSemaphoreGive(queueMUTEX);

    if (result == pdPASS){
      String message="{\"topic\":\""+String(toSend.topic)+"\",\"value\":"+String(toSend.value,2)+",\"timestamp\":"+toSend.timestamp+"}";
      Serial.println("TASK_M) sending: "+message+".");

      if (client.publish(String(toSend.topic).c_str(), message.c_str())){
        Serial.println("TASK_M) message (" + String(toSend.topic) + ") sent.");
      }else{
        Serial.println("TASK_M) failed to send message (" + String(toSend.topic) + "), checking connection.");
        connect_MQTT();
        Serial.println("TASK_M) trying to put the failed send message (" + String(toSend.topic) + ") back in list.");
        if (xSemaphoreTake(queueMUTEX, portMAX_DELAY) && uxQueueSpacesAvailable(messageQueue) != 0){
          // If the queue has still space, the message is restored
          xQueueSendToFront(messageQueue,&toSend,10000);
          xSemaphoreGive(queueMUTEX);
          Serial.println("TASK_M) the message (" + String(toSend.topic) + ") is again the first in list.");
        }else{
          Serial.println("TASK_M) the message (" + String(toSend.topic) + ") is no more in list beacuse of there were no space available.");
        }
      }
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  // Get payload as a string
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  // Print message
  Serial.println("CALLBA) message of \"" + String(topic) + "\" received: "+ String(message));

  // Acting
  if (String(topic)=="SetTime"){
    message.trim();
    char* token;
    const char* delimiters = ",/:-T";
    int values[6];
    int i = 0;
    token = strtok((char*)message.c_str(), delimiters);
    while (token != NULL && i < 6) {
      values[i++] = atoi(token);
      token = strtok(NULL, delimiters);
    }
    // if all the data are correctly found, and each data respect the right format, the rtc is corrected
    if (i==6 && 0<values[1]&&values[1]<=12 && 0<values[2]&&values[2]<=31 && 0<=values[3]&&values[3]<24 && 0<=values[4]&&values[4]<60 && 0<=values[5]&&values[5]<60){
      rtc.adjust(DateTime(values[0],values[1],values[2],values[3],values[4],values[5]));
      Serial.println("CALLBA) settled the following time: "+String(values[0])+"/"+String(values[1])+"/"+String(values[2])+" - "+String(values[3])+":"+String(values[4])+":"+String(values[5]));
    }else{
      Serial.println("CALLBA) the time is not in the right form, it is impossible to set the correct time.");
    }
  }
}

String pad0(int number) {
  return number<10? "0" + String(number):String(number);
}

String get_now(DateTime now){
  String time = "\""+String(now.year(), DEC)+"-"+pad0(now.month())+"-"+pad0(now.day())+" "+pad0(now.hour())+":"+pad0(now.minute())+":"+pad0(now.second())+"\"";
  return time;
}

void sendInBackQueue(String task,SingleData data){
    Serial.println(task+" trying send message in queue.");
  if(xSemaphoreTake(queueMUTEX, portMAX_DELAY)){
    if (uxQueueSpacesAvailable(messageQueue) == 0) {
      // The queue is full of message, the oldest is deleted to make space for the newest
      SingleData oldMessage;
      xQueueReceive(messageQueue, &oldMessage, 0);
      Serial.println(task+" The oldest message is removed to make space available for the new message.");
    }
    xQueueSendToBack(messageQueue, (void *)&data, pdMS_TO_TICKS(10000));
    Serial.println(task+" sent message in queue.");
    xSemaphoreGive(queueMUTEX);
  }
}

void TaskLUMINOSITY(void *pvParameters){
  TickType_t InitTime_L = xTaskGetTickCount();
  SingleData sdL = SingleData();
  strcpy(sdL.topic,topicL);
  DateTime nowL; 
  while (1){
    xSemaphoreTake(i2cMUTEX, portMAX_DELAY);
    sdL.value = lightMeter.readLightLevel();
    nowL = rtc.now();
    xSemaphoreGive(i2cMUTEX);

    strcpy(sdL.timestamp,get_now(nowL).c_str());
    Serial.println("TASK_L) read luminosity value: "+String(sdL.value)+".");
    sendInBackQueue("TASK_L)",sdL);

    // Run every Frequqnecy_L s
    vTaskDelayUntil( &InitTime_L, Frequency_L);
  }
}

void TaskPRESSURE(void *pvParameters){
  TickType_t InitTime_P = xTaskGetTickCount();
  SingleData sdP, sdA;
  DateTime nowP;
  strcpy(sdP.topic,topicP);
  strcpy(sdA.topic,topicA);
  while (1){
    xSemaphoreTake(i2cMUTEX, portMAX_DELAY);
    sdP.value = bmp.readPressure();
    nowP = rtc.now();
    xSemaphoreGive(i2cMUTEX);
    
    strcpy(sdP.timestamp,get_now(nowP).c_str());
    Serial.println("TASK_P) read pressure value: "+String(sdP.value)+".");
    sendInBackQueue("TASK_P)",sdP);
    
    sdA.value = 44330 * (1.0 - pow(sdP.value/100 / seaLevelhPa, 0.1903));
    strcpy(sdA.timestamp,get_now(nowP).c_str());
    Serial.println("TASK_P) calculated altitude value: "+String(sdA.value)+".");
    sendInBackQueue("TASK_P)",sdA);

    // Run every Frequency_P s
    vTaskDelayUntil( &InitTime_P, Frequency_P);
  }
}

void TaskTEMPERATURE(void *pvParameters){
  TickType_t InitTime_T = xTaskGetTickCount();
  SingleData sdT;
  DateTime nowT;
  strcpy(sdT.topic,topicT);
  while (1){
    xSemaphoreTake(i2cMUTEX, portMAX_DELAY);
    sdT.value = bmp.readTemperature();
    nowT=rtc.now();
    xSemaphoreGive(i2cMUTEX);

    strcpy(sdT.timestamp,get_now(nowT).c_str());
    Serial.println("TASK_T) read temperature value: "+String(sdT.value)+".");
    sendInBackQueue("TASK_T)",sdT);

    // Run every Frequency_T s
    vTaskDelayUntil( &InitTime_T, Frequency_T);
  }
}

void setup(){
  // SERIALE
  Serial.begin(9600);
  Serial.println("SETUP ) connected to serial.");

  // WIFI & MQTT
  client.setCallback(callback);
  connect_MQTT();
  Serial.println("SETUP ) initialized connection WiFi and MQTT.");

  // SENSORI
  RTC.begin(RTC_SDA,RTC_SCL, 100000);
  rtc.begin(&RTC);
  // Set the time only if necessary
  // rtc.adjust(DateTime(2023,6,19,22,33,0));
  I2C.begin(I2C_SDA, I2C_SCL, 100000);
  bmp = Adafruit_BMP280(&I2C);
  bmp.begin(0x77);
  lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, 0x23, &I2C);

  queueMUTEX = xSemaphoreCreateMutex();
  i2cMUTEX = xSemaphoreCreateMutex();

  messageQueue = xQueueCreate(QueueSize, sizeof(SingleData));
  Serial.println("SETUP ) initialized queue.");


  // Creazione dei task FreeRTOS
  xTaskCreate(    TaskMESSAGE,     "SendingMESSAGE", 10000, NULL, 1, NULL);
  xTaskCreate( TaskLUMINOSITY,  "ReadingLUMINOSITY", 10000, NULL, 3, NULL);
  xTaskCreate(   TaskPRESSURE,    "ReadingPRESSURE", 10000, NULL, 2, NULL);
  xTaskCreate(TaskTEMPERATURE, "ReadingTEMPERATURE", 10000, NULL, 2, NULL);
  Serial.println("SETUP ) tasks created.");
}

void loop() {
  client.loop();
}