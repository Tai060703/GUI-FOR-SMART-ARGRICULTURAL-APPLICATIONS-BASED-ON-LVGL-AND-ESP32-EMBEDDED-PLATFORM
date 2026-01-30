#ifndef NETWORK_SETUP_H
#define NETWORK_SETUP_H
#include "ui.h"
#include "Arduino.h"
#include "WiFi.h"
#include "WiFiClientSecure.h"
#include "PubSubClient.h"
#include "HTTPClient.h"
#include "ArduinoJson.h"
#include "esp_log.h"
#include "EEPROM.h"
#include "time.h"
#include <vector>
#include "SHT31.h"
#include "esp_now.h"
//#include "storage.h"

struct wifi_config {
    char ssid[30];
    char password[30];
    bool connected;
};
  
extern wifi_config wifi_setting;
extern const char* mqtt_server;
extern const int mqtt_port;
extern const char* mqtt_user;
extern const char* mqtt_password;

#define PUBLISH_BROKER_ADDRESS "app.coreiot.io"
#define PUBLISH_PORT 1883
#define PUBLISH_ACCESS_TOKEN "tranmanhtai0607"

// Topic Telemetry
#define TELEMETRY_TOPIC "v1/devices/me/telemetry"

// Khai báo client mới (hoặc tái sử dụng)
// extern WiFiClient publishWifiClient;
// extern PubSubClient publishMqttClient;


// Hàm khởi tạo và kết nối cho Publish Client
void setup_publish_mqtt();
void publish_telemetry_data(float temp, float humid);


bool wifi_init();
void setup_server();
void reconnect();
void network_run();
bool server_connected();
void SensorRun();
void setup_time();
void SensorBegin();
void httpRun();
bool publish_mqtt_connected();
void sync_all_attributes_to_server();
#endif 