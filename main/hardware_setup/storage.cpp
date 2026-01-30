#include "storage.h"
#include <Preferences.h> // Đảm bảo include ở đây

// Khai báo biến toàn cục (đã extern trong storage.h)
scheduler_event_t event_list[MAX_SCHEDULE_EVENTS];
size_t event_count = 0; 
Preferences preferences; // Đối tượng Preferences
// Khai báo biến toàn cục (đã extern trong storage.h)
selected_date_t g_selected_date = {0, 0, 0};
int32_t g_event_edit_index = -1;

switch_data switch_data_array[8];

// Khởi tạo NVS
void init_nvs_storage() {
    preferences.begin(NVS_NAMESPACE, false); // Mở namespace "scheduler"
}

// Lưu toàn bộ mảng Event vào NVS
bool save_event_list() {
    size_t data_size = event_count * sizeof(scheduler_event_t);
    
    // 1. Lưu số lượng Event
    preferences.putUInt("event_count", event_count);
    
    // 2. Lưu toàn bộ mảng Event
    // Sử dụng putBytes để lưu cả khối dữ liệu struct
    if (preferences.putBytes("events_data", event_list, data_size) > 0) {
        Serial.printf("NVS: Saved %u events, size: %u bytes.\n", event_count, data_size);
        return true;
    }
    Serial.println("NVS: ERROR saving event data.");
    return false;
}

// Tải toàn bộ mảng Event từ NVS
bool load_event_list() {
    // 1. Tải số lượng Event
    event_count = preferences.getUInt("event_count", 0); // Mặc định là 0
    
    // 2. Nếu có Event, tải mảng
    if (event_count > 0 && event_count <= MAX_SCHEDULE_EVENTS) {
        size_t data_size = event_count * sizeof(scheduler_event_t);
        size_t bytes_read = preferences.getBytes("events_data", event_list, data_size);
        
        if (bytes_read == data_size) {
            Serial.printf("NVS: Loaded %u events, size: %u bytes.\n", event_count, bytes_read);
            return true;
        }
    }
    
    // Nếu không có Event hoặc lỗi
    event_count = 0;
    Serial.println("NVS: No events or error loading data.");
    return false;
}

bool clearEEPROM() {
    for (int i = 0; i < 1024; i++) {
        EEPROM.write(i, 0xFF); // Write 0xFF to each byte of EEPROM
    }
    EEPROM.commit();
    Serial.println("EEPROM cleared");
    return true;
}

bool readSwitchData(switch_data *data) {
    for (int i = 0; i < 8; i++) {
        EEPROM.get(storage_address + i * sizeof(switch_data), data[i]); // Read the data from EEPROM
    }
    return true;
}

bool loadSwitchData(switch_data *data) {
    for(int i = 0; i < 8; i++) {
        EEPROM.put(storage_address + i * sizeof(switch_data), data[i]); // Write the data to EEPROM
        Serial.print(data[i].name);
        Serial.print(" ");
        Serial.print("State: ");
        Serial.print(data[i].state);
        Serial.print(" ");
        Serial.println(data[i].schedule);
        Serial.print("Schedule time: ");
        Serial.print(data[i].scheduletime.tm_year);
        Serial.print("-");
        Serial.print(data[i].scheduletime.tm_mon);
        Serial.print("-");
        Serial.println(data[i].scheduletime.tm_mday);
    }
    Serial.println("Switch data loaded to EEPROM");
    EEPROM.commit();
    return true;
}

void SwitchDataRun(){
    if(switch_data_changed){
        loadSwitchData(switch_data_array); // Save the switch data to EEPROM
        switch_data_changed = false;
    }
}

void EEPROMDataInit(){
    if(EEPROM.read(0) == 0xFF){
        wifi_setting.connected = false; // Set wifi_connected to false
    }
    else{
        EEPROM.get(0, wifi_setting);
        Serial.print("EEPROM data: ");
        Serial.print(wifi_setting.ssid);
        Serial.print(" ");
        Serial.print(wifi_setting.password);
        Serial.print(" ");
        Serial.println(wifi_setting.connected);
        if(wifi_setting.connected){
            if(!wifi_init()){
                Serial.println("Failed to connect to WiFi");
            }
        }
    }
    if(EEPROM.read(storage_address) != 0xFF){
        readSwitchData(switch_data_array); 
    }
}


scheduler_event_t g_current_active_event = {0}; // Khởi tạo bằng 0
bool g_is_event_running = false;
SemaphoreHandle_t g_event_mutex = NULL; // Sẽ được khởi tạo trong app_main
int32_t g_active_scheduler_index = -1;
scheduler_event_t g_temp_override_event = {0}; // Event tạm thời cho Override