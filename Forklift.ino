#include "Wire.h"
#include "WiFi.h"
#include "PubSubClient.h"
#include "time.h"
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include "time.h"


// Sensor2 UART
#define RXD1 18
#define TXD1 17

// Sensor1 UART
#define RXD2 16
#define TXD2 15

uint8_t dist1;
uint8_t layer;

uint8_t dist2;

bool hasRestarted = false;



unsigned char check; /*----save check value------------------------*/
int i;
unsigned char uart[9]; /*----save data measured by LiDAR-------------*/
const int HEADER = 0x59; /*----frame header of data package------------*/
int rec_debug_state = 0x01; //receive state for frame
const int OBJECT_DETECTION_THRESHOLD = 50;
bool isObjectDetected = false;

// WiFi
const char* ssid = "SSID"; 
const char* password =  "PASSWORD";

// MQTT Broker
const char *mqtt_broker = "mqttbrokername";
const char *topic = "Sensors/F4:12:FA:E3:A7:0C";  /*change with your topic name*/
const char *mqtt_username = "user_name";
const char *mqtt_password = "password";
const int mqtt_port = 10001;

WiFiClient espClient;
PubSubClient client(espClient);

AsyncWebServer server(80); // Create an AsyncWebServer instance



void setup() {
  delay(2000);
  Serial.begin(115200);

  // Initialize AsyncElegantOTA
  //AsyncElegantOTA.begin(&server); // Start the server for OTA updates
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { //WiFi
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");

   // Print the ESP32's IP address
  Serial.print("Connected to the WiFi network. IP address: ");
  Serial.println(WiFi.localIP());

  Serial1.begin(115200, SERIAL_8N1, RXD1, TXD1);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);

  // Connecting to MQTT broker
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
  while (!client.connected()) {
    String client_id = "esp32-client-";
    client_id += String(WiFi.macAddress());
    Serial.printf("The client %s connects to the public MQTT broker\n", client_id.c_str());
    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("Public EMQX MQTT broker connected");
    } else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }

    // Set the ESP32 time using NTP
    configTime(0, 0, "pool.ntp.org");  // Configure NTP time synchronization

    // Wait for time to be set
    time_t now;
    while ((now = time(nullptr)) < 1000000000) {
      delay(100);
    }
  }

  // Define OTA endpoints
  AsyncElegantOTA.begin(&server); // Enable OTA firmware updates
  server.begin();
  Serial.println("HTTP server started");

  // Set the ESP32 time using NTP
  configTime(7 * 3600, 0, "pool.ntp.org"); // Configure NTP time 
}

void callback(char *topic, byte *payload, unsigned int length) {   
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char) payload[i]);
  }
  Serial.println();
  Serial.println("-----------------------");
}

void loop() {
  //unsigned long currentTime = millis();
  // Apply the synchronized delay for both sensors
  //if (currentTime - lastReadTime >= sensorReadInterval) {
    //lastReadTime = currentTime;
    //delay(sensorDelay);

    if (!client.connected()) {
      reconnectMQTT();
    }

    if (WiFi.status() != WL_CONNECTED) {
      reconnectWiFi();
    }


    
    AsyncElegantOTA.loop();
    // Read Sensor1 data
    Get_Lidar_data1();

    // Read Sensor2 data
    Get_Lidar_data2();

    client.loop();
  //}

  // Add any other code or actions you want to perform independently here
   struct tm timeinfo;
   if(!getLocalTime(&timeinfo)){
     Serial.println("Failed to obtain time");
     return;
   }
   int currentHour = timeinfo.tm_hour;
   int currentMinute = timeinfo.tm_min;
   int currentSecond = timeinfo.tm_sec;

  // Check whether it's 3:00 AM and the ESP32 has not been restarted yet
   if (currentHour == 3 && currentMinute == 00 && currentSecond == 0) {
     Serial.println("Restarting ESP32...");
     ESP.restart();
     //hasRestarted = true;  // Prevents ESP.restart() from being called again
   }

  // Reset the flag at 3:01 AM to allow the ESP32 to restart again the next day
   if (currentHour == 3 && currentMinute == 00 && currentSecond == 1) {
     //hasRestarted = false;
   }

  // ... (rest of your loop code)
 

  
}


void reconnectWiFi() {
  Serial.println("Attempting to reconnect to WiFi...");
  WiFi.disconnect();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Reconnecting to WiFi...");
  }
  Serial.println("Reconnected to the WiFi network");
}

void reconnectMQTT() {
  static unsigned long lastReconnectAttempt = 0;
  unsigned long currentMillis = millis();

  if (currentMillis - lastReconnectAttempt >= 5000){
    lastReconnectAttempt = currentMillis;
  
    Serial.println("Attempting to reconnect to MQTT broker...");
    while (!client.connected()) {
      String client_id = "esp32-client-";
      client_id += String(WiFi.macAddress());
    
      if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
        Serial.println("MQTT broker reconnected");
        //lastReconnectAttempt = 0;  // reset the last reconnect attempt
        break;
      } else {
        Serial.println("MQTT connection failed, state: ");
        Serial.println(client.state());
        delay(2000);
      }
    }
  }
}

void Get_Lidar_data1() {
  if (Serial1.available() >= 9) {
    if (Serial1.read() == 0x59) {
      byte uart[9];
      uart[0] = 0x59;

      for (int i = 1; i < 9; i++) {
        uart[i] = Serial1.read();
      }

      unsigned char check = 0;
      for (int i = 0; i < 8; i++) {
        check += uart[i];
      }

      if (uart[8] == check) {
          dist1 = uart[2] + uart[3] * 256;

        }

        //if (newDist1 != dist1) {
          //dist1 = newDist1;

          Serial.print("Sensor1: ");
          Serial.print(" Distance1 = ");
          Serial.print(dist1);
          Serial.println();

          // Get MAC address
          String macAddress = WiFi.macAddress();
          time_t currentTimestamp = time(nullptr);
          char timestampString[11]; 


          // Prioritize layer detection
          if (dist1 <= 30) {
            layer = 1;
          } else if (dist1 >= 31 && dist1 <= 60) {
            layer = 2;
          } else if (dist1 > 60) {
            layer = 3;
          }
          const size_t maxPayloadSize = 50;
          char payload1[maxPayloadSize];
          String sensor1Topic = "Sensors/F4:12:FA:E3:A7:0C/Sensor1";
          
  
          int neededSize = snprintf(payload1, maxPayloadSize, "{\"height\":%u,\"floor\":%u,\"timestamp\":%lu}", dist1, layer, millis());

          

          // //check the declared size is fine or not so that we can declare the best variable type to send data faster
          if (neededSize >= maxPayloadSize) {
              Serial.println("Payload buffer is too small!");
              // Handle the error, perhaps send an error message instead
          }
          else {
              Serial.println("Payload size is fine!");
              client.publish(sensor1Topic.c_str(), payload1, 0);
        }
      }
    }
  }

void Get_Lidar_data2() {
  if (Serial2.available() >= 9) {
    if (Serial2.read() == 0x59) {
      byte uart[9];
      uart[0] = 0x59;

      for (int i = 1; i < 9; i++) {
        uart[i] = Serial2.read();
      }

      unsigned char check = 0;
      for (int i = 0; i < 8; i++) {
        check += uart[i];
      }

      if (uart[8] == check) {
            dist2 = uart[2] + uart[3] * 256;

          }


        //if (newDist2 != dist2) {
          //dist2 = newDist2;
          Serial.print("Sensor2: ");
          Serial.print(" Distance2 = ");
          Serial.print(dist2);
          Serial.println();
          // Get MAC address
          String macAddress = WiFi.macAddress();

          // Get current timestamp
          time_t currentTimestamp = time(nullptr);
          char timestampString[11]; 

          // Check for object detection
          if (dist2 <= OBJECT_DETECTION_THRESHOLD) {
            isObjectDetected = true;
          } else {
            isObjectDetected = false;
          }

          // Publish LiDAR data to MQTT broker
          const size_t maxPayloadSize = 50;
          char payload2[maxPayloadSize];
          String sensor2Topic = "Sensors/F4:12:FA:E3:A7:0C/Sensor2";
          
          int neededSize = snprintf(payload2, maxPayloadSize, "{\"object_detected\":%s,\"timestamp\":%lu}", isObjectDetected ? "true" : "false", millis());

          if (neededSize >= maxPayloadSize) {
              Serial.println("Payload buffer for Sensor2 is too small!");
              // Handle the error, perhaps send an error message instead
          }
          else {
              Serial.println("Payload size for Sensor2 is fine!");
              client.publish(sensor2Topic.c_str(), payload2, 0);
          }
        }
      }
    }
