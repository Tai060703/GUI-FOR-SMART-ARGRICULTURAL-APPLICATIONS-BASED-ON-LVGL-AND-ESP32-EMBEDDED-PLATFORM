#include "network_setup.h"
#include "storage.h"

WiFiClient wifiClient;
WiFiClientSecure wifiClientSecure2;
PubSubClient client(wifiClient);
HTTPClient httpClient;

WiFiClient publishWifiClient;
PubSubClient publishMqttClient(publishWifiClient);
 

wifi_config wifi_setting;

const char* ca_cert = R"EOF(-----BEGIN CERTIFICATE-----
MIIDiTCCAw+gAwIBAgISBRxgikly0Yfls23xsJF+KPZYMAoGCCqGSM49BAMDMDIx
CzAJBgNVBAYTAlVTMRYwFAYDVQQKEw1MZXQncyBFbmNyeXB0MQswCQYDVQQDEwJF
NjAeFw0yNTA1MTUxMTM1NDZaFw0yNTA4MTMxMTM1NDVaMBkxFzAVBgNVBAMTDmFw
cC5jb3JlaW90LmlvMFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAE+ODDmNhJn4LP
Ha3vvLI8p0eHxGX8sVObY2MewfKp+Q75zv6Hbi5fmX7W3ng5v6VBiWjJu5tlioKz
36Bapvx3y6OCAhwwggIYMA4GA1UdDwEB/wQEAwIHgDAdBgNVHSUEFjAUBggrBgEF
BQcDAQYIKwYBBQUHAwIwDAYDVR0TAQH/BAIwADAdBgNVHQ4EFgQUrpRZVXvVYJvb
ivae47I5zCSaghswHwYDVR0jBBgwFoAUkydGmAOpUWiOmNbEQkjbI79YlNIwMgYI
KwYBBQUHAQEEJjAkMCIGCCsGAQUFBzAChhZodHRwOi8vZTYuaS5sZW5jci5vcmcv
MBkGA1UdEQQSMBCCDmFwcC5jb3JlaW90LmlvMBMGA1UdIAQMMAowCAYGZ4EMAQIB
MC0GA1UdHwQmMCQwIqAgoB6GHGh0dHA6Ly9lNi5jLmxlbmNyLm9yZy84OS5jcmww
ggEEBgorBgEEAdZ5AgQCBIH1BIHyAPAAdQDtPEvW6AbCpKIAV9vLJOI4Ad9RL+3E
hsVwDyDdtz4/4AAAAZbT8DRRAAAEAwBGMEQCIDT+Fjh9nWDocCF5YF3o3qQVMfWT
TTc+gWT5aw4HOBWEAiAJDYj7O6Y7I3d2NZ9ZlgLxKYkt6+i/hQZ0w//TkxhLyAB3
AA3h8jAr0w3BQGISCepVLvxHdHyx1+kw7w5CHrR+Tqo0AAABltPwNF4AAAQDAEgw
RgIhAJpX2vaV9efuKqsiHRVx3muV51CLIBUM+knCluAbvrs7AiEAr+PPry8HafLx
fEIurS7Qt+f1YlIUb6Lq1EFaIIQyQHgwCgYIKoZIzj0EAwMDaAAwZQIwbhX/pv7h
VcN86gOgDo2Zso2bGL9PN1pkKHNol45IggQrnGbm3POKIPIK9QA+gv4JAjEAzlA9
92dXlycn3BOb4z94LRrGCAeOwvKqk6bg+pt07P4jU71cohRk6VsfrsLSf3b6
-----END CERTIFICATE-----
-----BEGIN CERTIFICATE-----
MIIEVzCCAj+gAwIBAgIRALBXPpFzlydw27SHyzpFKzgwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMjQwMzEzMDAwMDAw
WhcNMjcwMzEyMjM1OTU5WjAyMQswCQYDVQQGEwJVUzEWMBQGA1UEChMNTGV0J3Mg
RW5jcnlwdDELMAkGA1UEAxMCRTYwdjAQBgcqhkjOPQIBBgUrgQQAIgNiAATZ8Z5G
h/ghcWCoJuuj+rnq2h25EqfUJtlRFLFhfHWWvyILOR/VvtEKRqotPEoJhC6+QJVV
6RlAN2Z17TJOdwRJ+HB7wxjnzvdxEP6sdNgA1O1tHHMWMxCcOrLqbGL0vbijgfgw
gfUwDgYDVR0PAQH/BAQDAgGGMB0GA1UdJQQWMBQGCCsGAQUFBwMCBggrBgEFBQcD
ATASBgNVHRMBAf8ECDAGAQH/AgEAMB0GA1UdDgQWBBSTJ0aYA6lRaI6Y1sRCSNsj
v1iU0jAfBgNVHSMEGDAWgBR5tFnme7bl5AFzgAiIyBpY9umbbjAyBggrBgEFBQcB
AQQmMCQwIgYIKwYBBQUHMAKGFmh0dHA6Ly94MS5pLmxlbmNyLm9yZy8wEwYDVR0g
BAwwCjAIBgZngQwBAgEwJwYDVR0fBCAwHjAcoBqgGIYWaHR0cDovL3gxLmMubGVu
Y3Iub3JnLzANBgkqhkiG9w0BAQsFAAOCAgEAfYt7SiA1sgWGCIpunk46r4AExIRc
MxkKgUhNlrrv1B21hOaXN/5miE+LOTbrcmU/M9yvC6MVY730GNFoL8IhJ8j8vrOL
pMY22OP6baS1k9YMrtDTlwJHoGby04ThTUeBDksS9RiuHvicZqBedQdIF65pZuhp
eDcGBcLiYasQr/EO5gxxtLyTmgsHSOVSBcFOn9lgv7LECPq9i7mfH3mpxgrRKSxH
pOoZ0KXMcB+hHuvlklHntvcI0mMMQ0mhYj6qtMFStkF1RpCG3IPdIwpVCQqu8GV7
s8ubknRzs+3C/Bm19RFOoiPpDkwvyNfvmQ14XkyqqKK5oZ8zhD32kFRQkxa8uZSu
h4aTImFxknu39waBxIRXE4jKxlAmQc4QjFZoq1KmQqQg0J/1JF8RlFvJas1VcjLv
YlvUB2t6npO6oQjB3l+PNf0DpQH7iUx3Wz5AjQCi6L25FjyE06q6BZ/QlmtYdl/8
ZYao4SRqPEs/6cAiF+Qf5zg2UkaWtDphl1LKMuTNLotvsX99HP69V2faNyegodQ0
LyTApr/vT01YPE46vNsDLgK+4cL6TrzC/a4WcmF5SRJ938zrv/duJHLXQIku5v0+
EwOy59Hdm0PT/Er/84dDV0CSjdR/2XuZM3kpysSKLgD1cKiDA+IRguODCxfO9cyY
Ig46v9mFmBvyH04=
-----END CERTIFICATE-----
)EOF";

const char* mqtt_server = "app.coreiot.io";
const char* http_server = "https://app.coreiot.io";
const int mqtt_port = 1883;
const char* mqtt_user = "iot_device_2";
const char* mqtt_password = "DAKTMT_NHOM5";
const long gmtOffset_sec = 7 * 3600;
const int daylightOffset_sec = 0;



SHT31 sht(0x44, I2C_NUM_0);
float temperature = 0;
float humidity = 0;

String message;
std::vector<lv_coord_t> temperature_data;
std::vector<lv_coord_t> humidity_data;

void setup_time() {
    configTime(gmtOffset_sec, daylightOffset_sec, "pool.ntp.org");
    Serial.println("Time setup done");
}


bool wifi_init(){
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_setting.ssid);

  WiFi.disconnect(true);
  delay(200);
  WiFi.begin(wifi_setting.ssid, wifi_setting.password);
  int time = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - time < 10000) {
    delay(200);
    Serial.print(".");
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Failed to connect to WiFi");
    return false;
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  return true;
}
void sync_all_attributes_to_server() {
    if (!publishMqttClient.connected()) return;

    if (xSemaphoreTake(g_event_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
        StaticJsonDocument<512> doc;
        
        // Ánh xạ từ mảng 0:LIGHT, 1:SPRAYER, 2:PUMP, 3:FAN
        doc["lightState"] = g_current_active_event.device_states[0];
        doc["lightValue"] = g_current_active_event.device_levels[0];
        
        doc["sprayerState"] = g_current_active_event.device_states[1];
        doc["sprayerValue"] = g_current_active_event.device_levels[1];
        
        doc["pumpState"] = g_current_active_event.device_states[2];
        doc["pumpValue"] = g_current_active_event.device_levels[2];
        
        doc["fanState"] = g_current_active_event.device_states[3];
        doc["fanValue"] = g_current_active_event.device_levels[3];

        char buffer[512];
        size_t n = serializeJson(doc, buffer);
        publishMqttClient.publish("v1/devices/me/attributes", buffer, n);
        
        xSemaphoreGive(g_event_mutex);
        Serial.println("MQTT: Attributes synced to Server 2.");
    }
}
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Received from: ");
  Serial.println(topic);
    
    // Đảm bảo payload là chuỗi kết thúc null (null-terminated)
    char json_buffer[length + 1];
    memcpy(json_buffer, payload, length);
    json_buffer[length] = '\0';
    
    // --- 1. Phân tích JSON ---
    StaticJsonDocument<512> doc; // Điều chỉnh kích thước nếu cần
    DeserializationError error = deserializeJson(doc, json_buffer);

    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
    }

    const char* method = doc["method"];
    const char* params_str = doc["params"];
    
    if (method && strcmp(method, "data") == 0 && params_str) {
        
        Serial.print("Payload params: ");
        Serial.println(params_str);
        
        float temp = 0.0f;
        float humid = 0.0f;
        
        // --- 2. Trích xuất 2 giá trị đầu tiên từ chuỗi params ---
        
        // Trích xuất bằng sscanf (cách dễ nhất cho các chuỗi có định dạng cố định)
        // Chúng ta chỉ cần lấy 2 giá trị float đầu tiên.
        if (sscanf(params_str, "%f_%f_", &temp, &humid) == 2) { 
            
            Serial.printf("Extracted: Temp=%.1f, Humid=%.1f\n", temp, humid);
            
            // --- 3. Cập nhật UI ---
            // Gọi hàm LVGL (cần đảm bảo chạy trong context thread-safe nếu có)
            // Vì đây là PubSubClient, nó thường chạy trên Core 1 hoặc Main Task.
            char temp_str[15];
            char humid_str[15];

            // Định dạng chuỗi cho Nhiệt độ và Độ ẩm
            sprintf(temp_str, "%.1f C", temp);
            sprintf(humid_str, "%.1f %%", humid);
            temperature_data.push_back(temp);
            humidity_data.push_back(humid);
            if(temperature_data.size() > 10) {
              temperature_data.erase(temperature_data.begin());
            }
            if(humidity_data.size() > 10) {
              humidity_data.erase(humidity_data.begin());
            }
            // Cập nhật Label trên Screen 1
            if (ui_TempLabel != NULL) {
                lv_label_set_text(ui_TempLabel, temp_str);
            }
            if (ui_HumidLabel != NULL) {
                lv_label_set_text(ui_HumidLabel, humid_str);
            } 
            lv_chart_set_point_count(ui_Chart2, temperature_data.size());
            lv_chart_set_ext_y_array(ui_Chart2, ui_Chart2_series_1, temperature_data.data());
            lv_chart_set_ext_y_array(ui_Chart2, ui_Chart2_series_2, humidity_data.data());
            publish_telemetry_data(temp, humid);
            
        } else {
            Serial.println("Error extracting sensor data from params string.");
        }
    }
    
    // Logic xử lý RPC hoặc các method khác (nếu cần)
    if (strstr(topic, "/rpc/request/") != NULL) {
        // ... (Logic RPC response)
    }
}
void publish_mqtt_callback(char* topic, byte* payload, unsigned int length) {
    char json_buffer[length + 1];
    memcpy(json_buffer, payload, length);
    json_buffer[length] = '\0';

    StaticJsonDocument<256> doc;
    if (deserializeJson(doc, json_buffer)) return;

    const char* method = doc["method"];
    JsonVariant params = doc["params"];
    
    // Trích xuất Request ID từ topic để phản hồi RPC
    char* request_id = strrchr(topic, '/') + 1;
    char response_topic[64];
    snprintf(response_topic, sizeof(response_topic), "v1/devices/me/rpc/response/%s", request_id);

    bool handled = false;
    bool need_disable_event = false;
    JsonVariant response_val;

    if (xSemaphoreTake(g_event_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        // Xử lý lệnh SET và GET tương tự logic Python của bạn
        if (strcmp(method, "setLightState") == 0) {
            g_current_active_event.device_states[0] = params.as<bool>();
            response_val = params;
            handled = true;
            need_disable_event = true;
        } else if (strcmp(method, "setLightValue") == 0) {
            g_current_active_event.device_levels[0] = params.as<uint8_t>();
            response_val = params;
            handled = true;
            need_disable_event = true;
        } else if (strcmp(method, "setFanState") == 0) {
            g_current_active_event.device_states[3] = params.as<bool>();
            response_val = params;
            handled = true;
            need_disable_event = true;
        } else if (strcmp(method, "setFanValue") == 0) {
            g_current_active_event.device_levels[3] = params.as<uint8_t>();
            response_val = params;
            handled = true;
            need_disable_event = true;
        }
        else if (strcmp(method, "setPumpState") == 0) {
            g_current_active_event.device_states[2] = params.as<bool>();
            response_val = params;
            handled = true;
            need_disable_event = true;
        } else if (strcmp(method, "setPumpValue") == 0) {
            g_current_active_event.device_levels[2] = params.as<uint8_t>();
            response_val = params;
            handled = true;
            need_disable_event = true;
        }
         else if (strcmp(method, "setSprayerState") == 0) {
            g_current_active_event.device_states[1] = params.as<bool>();
            response_val = params;
            handled = true;
            need_disable_event = true;
        } else if (strcmp(method, "setSprayerValue") == 0) {
            g_current_active_event.device_levels[1] = params.as<uint8_t>();
            response_val = params;
            handled = true;
            need_disable_event = true;
        }
        // Thêm tương tự cho Pump và Sprayer...


        xSemaphoreGive(g_event_mutex);
       
    }
    if (handled && need_disable_event && g_is_event_running) {
            if (g_active_scheduler_index != -1 && g_active_scheduler_index < event_count) {
                
                // 1. Disable event gốc trong danh sách
                event_list[g_active_scheduler_index].is_enabled = false; 
                
                // 2. Lưu thay đổi vào NVS
                if (save_event_list()) {
                    Serial.printf("MQTT OVERRIDE: Disabled scheduled event '%s' at index %d\n", 
                                  event_list[g_active_scheduler_index].label, g_active_scheduler_index);
                }

                // 3. Đánh dấu event scheduler không còn chạy (để chuyển sang chế độ manual hoàn toàn)
                g_is_event_running = false;
                g_active_scheduler_index = -1;
            }
        }
         update_screen2_ui(); // Cập nhật UI sau khi thay đổi trạng thái
    if (handled) {
        char resp_buf[64];
        serializeJson(response_val, resp_buf);
        publishMqttClient.publish(response_topic, resp_buf);
        
        // Sau khi thay đổi trạng thái từ server, đồng bộ lại dashboard
        sync_all_attributes_to_server();
    }
}
void setup_server(){
  // wifiClientSecure2.setCACert(ca_cert);
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void reconnect() {
  unsigned int start_time = millis();
  while (!client.connected() && millis() - start_time < 10000) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32Client", mqtt_password, nullptr)) {
      Serial.println("connected");
      client.subscribe("v1/devices/me/rpc/request/+");
      Serial.println("Subscribed to RPC topic");
      client.subscribe("v1/devices/me/attributes");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(1000);
    }
  }
}

bool server_connected(){
  setup_server();
  reconnect();
  return true;
}

// Định nghĩa Global Client
// WiFiClient publishWifiClient;
// PubSubClient publishMqttClient(publishWifiClient);

void reconnect_publish_mqtt() {
    while (!publishMqttClient.connected()) {
        Serial.print("Attempting PUBLISH MQTT connection...");
        
        // Kết nối bằng PUBLISH_ACCESS_TOKEN
        if (publishMqttClient.connect("ESP32Client", PUBLISH_ACCESS_TOKEN, nullptr)) {
            Serial.println("Publish connected");
            publishMqttClient.subscribe("v1/devices/me/rpc/request/+");
            Serial.println("Subscribed to RPC topic");
            publishMqttClient.subscribe("v1/devices/me/attributes");
          } else {
            Serial.print("failed, rc=");
            Serial.print(publishMqttClient.state());
            Serial.println(" try again in 5 seconds");
            delay(1000);
          }
    }
}

void setup_publish_mqtt() {
    publishMqttClient.setServer(mqtt_server, mqtt_port);
    publishMqttClient.setCallback(publish_mqtt_callback);
    // Client này không cần callback vì nó chỉ gửi dữ liệu
}
bool publish_mqtt_connected(){
  setup_publish_mqtt();
  reconnect_publish_mqtt();
  return true;
}
void publish_telemetry_data(float temp, float humid) {
    if (publishMqttClient.connected()) {
        
        // Sử dụng DynamicJsonDocument vì payload có thể thay đổi
        String payload = "{\"temperature\":" + String(temp) +  ",\"humidity\":" + String(humid) + "}";
        
        publishMqttClient.publish("v1/devices/me/telemetry", payload.c_str());
        //client.publish("esp/telemetry", payload.c_str());
    }
    else {
        Serial.println("[PUBLISH] ERROR: MQTT not connected.");
        //reconnect_publish_mqtt();
    }
}

void MqttPublisherLoopTask(void * parameter) {
    setup_publish_mqtt();
    
    while(1) {
        if (WiFi.status() == WL_CONNECTED) {
            // 1. Duy trì kết nối Publish
            if (!publishMqttClient.connected()) {
                reconnect_publish_mqtt();
            }
            
            // 2. CHẠY LOOP để xử lý keep-alive và gửi/nhận packet
            publishMqttClient.loop();
        }
        
        // Delay ngắn (ví dụ: 50ms) để đảm bảo loop chạy thường xuyên mà không chặn core
        vTaskDelay(pdMS_TO_TICKS(50)); 
    }
}

void network_run(){
  
  
  if(wifi_setup_done){
    const char * text1 = lv_textarea_get_text(ui_SSIDText);
    const char * text2 = lv_textarea_get_text(ui_PasswordText);
    strcpy(wifi_setting.ssid, text1);
    strcpy(wifi_setting.password, text2);
    wifi_setup_done = false;
    wifi_setting.connected = true;

    EEPROM.put(0, wifi_setting);
    EEPROM.commit();
    bool wifi_status = wifi_init();
    if (wifi_status && lv_obj_has_flag(ui_WifiLoadingPanel, LV_OBJ_FLAG_HIDDEN) == false) {
      _ui_flag_modify(ui_WifiLoadingPanel, LV_OBJ_FLAG_HIDDEN, _UI_MODIFY_FLAG_ADD);
      _ui_screen_change(&ui_Screen1, LV_SCR_LOAD_ANIM_FADE_ON, 200, 0, &ui_Screen1_screen_init);
    }
    else if(wifi_status == false && lv_obj_has_flag(ui_WifiLoadingPanel, LV_OBJ_FLAG_HIDDEN) == false) {
      _ui_flag_modify(ui_WifiLoadingPanel, LV_OBJ_FLAG_HIDDEN, _UI_MODIFY_FLAG_ADD);
      lv_label_set_text(ui_WifiWarningLabel, "   Failed to connect to WiFi!");
      _ui_flag_modify(ui_WifiWarningType, LV_OBJ_FLAG_HIDDEN, _UI_MODIFY_FLAG_REMOVE);
    }
  }
  if (!client.connected()) {
    reconnect();
  }
  else if(client.connected()){
    //Serial.println("MQTT connected");
    client.loop();
  }
  if (!publishMqttClient.connected()) {
    reconnect_publish_mqtt();
  }
  else if (publishMqttClient.connected()) {
    //Serial.println("PUBLISH MQTT connected");
    publishMqttClient.loop();
  }
  //Serial.println("Network run");
}

void SensorBegin(){
  sht.begin();
  uint16_t stat = sht.readStatus();
  Serial.print(stat, HEX);
  Serial.println("Sensor begin success");
}

void httpRun(){
  if (WiFi.status() == WL_CONNECTED) {
    httpClient.begin(wifiClientSecure2, http_server); 

    int httpCode = httpClient.GET();

    if (httpCode > 0) {
      Serial.printf("HTTPS Response code: %d\n", httpCode);
      if (httpCode == HTTP_CODE_OK) {
        String payload = httpClient.getString();
        Serial.println("Response:");
        Serial.println(payload);
      }
    } else {
      Serial.printf("HTTPS GET failed, error: %s\n", httpClient.errorToString(httpCode).c_str());
    }
    httpClient.end();
  }
}

void SensorRun(){
  // int value = analogRead(6);
  // Serial.print("Analog value: ");
  // Serial.println(value);
  sht.read();

  temperature = sht.getTemperature();
  humidity = sht.getHumidity();
  char temp[30];
  char hum[25];
  sprintf(temp,"%.1f", temperature);
  sprintf(hum,"%.1f", humidity);
  temperature_data.push_back(temperature);
  humidity_data.push_back(humidity);
  if(temperature_data.size() > 10) {
    temperature_data.erase(temperature_data.begin());
  }
  if(humidity_data.size() > 10) {
    humidity_data.erase(humidity_data.begin());
  }
  lv_label_set_text(ui_TempLabel, temp);
  lv_label_set_text(ui_HumidLabel, hum);
  lv_chart_set_point_count(ui_Chart2, temperature_data.size());
  lv_chart_set_ext_y_array(ui_Chart2, ui_Chart2_series_1, temperature_data.data());
  lv_chart_set_ext_y_array(ui_Chart2, ui_Chart2_series_2, humidity_data.data());
  if(client.connected()){
    StaticJsonDocument<200> doc;
    doc["temperature"] = temperature;
    doc["humidity"] = humidity;
    char buffer[200];
    size_t n = serializeJson(doc, buffer);
    client.publish("v1/devices/me/telemetry", buffer, n);
  }
}

