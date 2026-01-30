#include "Arduino.h"
#include "EEPROM.h"
#include "ui.h"
#include "network_setup.h"
#include "time.h"
#include "Preferences.h" // <<< Thêm thư viện Preferences

// Giới hạn Event
#define MAX_SCHEDULE_EVENTS 100
#define NVS_NAMESPACE "scheduler" // Namespace NVS cho các Event

typedef struct {
    char label[32];         // Ví dụ: "Morning Watering"
    uint8_t start_hour;
    uint8_t start_minute;
    uint8_t end_hour;
    uint8_t end_minute;
    uint16_t year;
    uint8_t month; // 1-12
    uint8_t day;   // 1-31
    bool repeat_weekly;
    uint8_t active_days;    // Bitmask cho các ngày trong tuần (Mon, Tue, ...)
    bool device_states[4];  // LIGHT, SPRAYER, PUMP, FAN (On/Off)
    uint8_t device_levels[4];// LIGHT, SPRAYER, PUMP, FAN (0-100% từ Slider)
    bool is_enabled;        // Bật/Tắt Event
} scheduler_event_t;

// Khai báo mảng Event (sẽ được tải từ NVS khi khởi động)
extern scheduler_event_t event_list[MAX_SCHEDULE_EVENTS];
extern size_t event_count; // Số lượng event hiện có

extern scheduler_event_t g_current_active_event;
extern bool g_is_event_running; // Cờ báo hiệu có Event đang chạy hay không
extern SemaphoreHandle_t g_event_mutex; // Mutex bảo vệ g_current_active_event

const unsigned int storage_address = 62;

typedef struct {
    uint16_t year;
    uint8_t month;
    uint8_t day;
} selected_date_t;

struct switch_data {
    bool state;
    char name[20];
    char schedule[30] = "No Schedule Event"; 
    tm scheduletime = {0, 0, 0, 0, 0, 0, 0, 0, 0}; // Initialize to zero
}; // Array to hold switch data

extern switch_data switch_data_array[8]; // Declare the array of switch_data

bool loadSwitchData(switch_data *data);
bool readSwitchData(switch_data *data);
bool clearEEPROM();
void SwitchDataRun();
void EEPROMDataInit();

// Hàm NVS mới cho Scheduler
void init_nvs_storage(); // Khởi tạo NVS
bool save_event_list(); // Lưu toàn bộ mảng Event vào NVS
bool load_event_list(); // Tải toàn bộ mảng Event từ NVS

bool is_event_active_on_selected_date(const scheduler_event_t *event, const selected_date_t *selected_date);


// Biến toàn cục để lưu trữ bối cảnh giữa các màn hình
extern selected_date_t g_selected_date; // Ngày được chọn trên Screen 6
extern int32_t g_event_edit_index;      // Index của Event đang được Sửa/Xem (-1: Thêm mới)
extern int32_t g_active_scheduler_index; // Index của Event đang chạy trong event_list (-1 nếu không chạy)
extern scheduler_event_t g_temp_override_event; // Event tạm thời cho Override