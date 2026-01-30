/*
 * SPDX-FileCopyrightText: 2022-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "lvgl_setup.h"
#include "hardware_setup.h"
#include "network_setup.h"
#include "storage.h"
#include "PCF8574.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "Arduino.h"  
#include "WiFi.h"

#include "driver/i2c.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"



void networkTask(void * parameter) {
    while (WiFi.status() != WL_CONNECTED && wifi_setup_done == false) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    if(WiFi.status() == WL_CONNECTED) {
        _ui_flag_modify(ui_WifiNoti1, LV_OBJ_FLAG_HIDDEN, _UI_MODIFY_FLAG_REMOVE);
        _ui_flag_modify(ui_WifiNoti2, LV_OBJ_FLAG_HIDDEN, _UI_MODIFY_FLAG_REMOVE);
        _ui_flag_modify(ui_WifiNoti6, LV_OBJ_FLAG_HIDDEN, _UI_MODIFY_FLAG_REMOVE);
        _ui_flag_modify(ui_WifiNoti7, LV_OBJ_FLAG_HIDDEN, _UI_MODIFY_FLAG_REMOVE);
        server_connected();
        publish_mqtt_connected();
    }
    while (1){
        network_run();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void httpTask(void * parameter) {
    while(1) {
        httpRun();
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void I2CAddressTask(void * parameter) {
    Serial.println("I2C Address Scanner");
    while (1) {
        esp_err_t err;
        int deviceCount = 0;

        for (uint8_t address = 1; address < 127; address++) {
            i2c_cmd_handle_t cmd = i2c_cmd_link_create();
            i2c_master_start(cmd);
            i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_WRITE, true);
            i2c_master_stop(cmd);

            err = i2c_master_cmd_begin((i2c_port_t)I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000)); // Timeout in ms
            i2c_cmd_link_delete(cmd);
            if (err == ESP_OK) {
                Serial.print("Found I2C device at address 0x");
                Serial.println(address, HEX);
                deviceCount++;
            } else if (err == ESP_ERR_TIMEOUT) {
                Serial.print("I2C device at address 0x");
                Serial.print(address, HEX);
                Serial.println(" not responding");
            } else {
                Serial.print("Error: ");
                Serial.println(esp_err_to_name(err));
            }
        }

        if (deviceCount == 0) {
            Serial.println("No I2C devices found");
        } else {
            Serial.print("I2C devices found: ");
            Serial.println(deviceCount);
        }

        vTaskDelay(pdMS_TO_TICKS(5000));  // Delay for 5 seconds
    }
}

void SwitchDataTask(void *pvParameters)
{
    while (1)
    {
        SwitchDataRun();
        vTaskDelay(pdMS_TO_TICKS(1000)); // Delay for 1 second
    }
}

void readCommandlineTask(void *pvParameters)
{
    while (1)
    {
        char command[100];
        if (Serial.available() > 0) {
            Serial.readBytesUntil('\n', command, sizeof(command));
            Serial.print("Command received: ");
            Serial.println(command);
            if(strcmp(command, "clear") == 0) {
                clearEEPROM();
            }
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay for 1 second
    }
}

void SensorTask(void *pvParameters)
{
    SensorBegin();
    while (1)
    {
        // Simulate sensor data
        SensorRun();
        vTaskDelay(pdMS_TO_TICKS(2000)); // Delay for 2 seconds
    }
}

void GetTimeTask(void *pvParameters)
{
    bool schedule_executed = false;
    struct tm timeinfo;
    while (WiFi.status() != WL_CONNECTED || !getLocalTime(&timeinfo)){
        Serial.println("Failed to obtain time");
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    lv_calendar_set_showed_date(ui_Calendar3, timeinfo.tm_year + 1900, timeinfo.tm_mon + 1);
    
     while(1){
        if(WiFi.status() == WL_CONNECTED) {
            if (getLocalTime(&timeinfo)) {
                Serial.printf("Date: %04d-%02d-%02d\n", timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday);
                lv_calendar_set_today_date(ui_Calendar3, timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday);
            char date_str[20];
            lv_calendar_date_t selected_date[1];
            selected_date[0].year = timeinfo.tm_year + 1900;
            selected_date[0].month = timeinfo.tm_mon + 1;
            selected_date[0].day = timeinfo.tm_mday;
            sprintf(date_str, "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);

            lv_calendar_set_highlighted_dates(ui_Calendar3, selected_date, 1);
            lv_label_set_text(ui_TimeLabel1, date_str);
            lv_label_set_text(ui_TimeLabel2, date_str);
            lv_label_set_text(ui_TimeLabel5, date_str);
            lv_label_set_text(ui_TimeLabel6, date_str);
            lv_label_set_text(ui_TimeLabel7, date_str);
            const char* day_name = get_day_name_top_bar(timeinfo.tm_wday);
            
            const char* month_name = get_month_name_top_bar(timeinfo.tm_mon + 1);
            int day_of_month = timeinfo.tm_mday;
            sprintf(date_str, "%s, %s %02d", 
                    day_name, 
                    month_name, 
                    day_of_month);
            lv_label_set_text(ui_DateLabel5, date_str);
            lv_label_set_text(ui_DateLabel1, date_str);
            lv_label_set_text(ui_DateLabel2, date_str);
            lv_label_set_text(ui_DateLabel6, date_str);
            lv_label_set_text(ui_DateLabel7, date_str);
            
            const char* day_name_full = get_day_name(timeinfo.tm_wday);
                        char full_date_str[35]; 
                        
                        sprintf(full_date_str, "%s %02d/%02d/%d", 
                                day_name_full,
                                timeinfo.tm_mday,             
                                timeinfo.tm_mon + 1,          
                                timeinfo.tm_year + 1900);     
                        
                        if (ui_Date2 != NULL) { 
                            lv_label_set_text(ui_Date2, full_date_str);
                        }
            } else {
                Serial.println("Failed to obtain time");
            }
        }
        // --- Scheduler Task ---
        selected_date_t current_date = { (uint16_t)(timeinfo.tm_year + 1900), 
                                         (uint8_t)(timeinfo.tm_mon + 1), 
                                         (uint8_t)timeinfo.tm_mday };
        
        scheduler_event_t * next_active_event = NULL;
        int32_t next_active_event_index = -1;
        
        load_event_list(); // Đảm bảo Event List là mới nhất

        // --- VÒNG LẶP DUYỆT NGƯỢC (Cleanup và Tìm kiếm) ---
        // Duyệt ngược để đảm bảo an toàn khi DISABLE event, và ưu tiên event có index cao hơn (nếu có nhiều event trùng nhau)
        for (int i = event_count - 1; i >= 0; i--) {
            scheduler_event_t * event = &event_list[i];

            // Bỏ qua event nếu đã bị disable
            if (!event->is_enabled) {
                continue;
            }

            // --- 1. CLEANUP (Disable Event một lần đã quá hạn) ---
            if (!event->repeat_weekly) {
                uint64_t event_end_time = (uint64_t)event->year * 100000000 + 
                                          (uint64_t)event->month * 1000000 + 
                                          (uint64_t)event->day * 10000 + 
                                          (uint64_t)event->end_hour * 100 + 
                                          (uint64_t)event->end_minute;
                
                uint64_t current_time_val = (uint64_t)(timeinfo.tm_year + 1900) * 100000000 + 
                                            (uint64_t)(timeinfo.tm_mon + 1) * 1000000 + 
                                            (uint64_t)timeinfo.tm_mday * 10000 + 
                                            (uint64_t)timeinfo.tm_hour * 100 + 
                                            (uint64_t)timeinfo.tm_min;

                if (current_time_val >= event_end_time) {
                    
                    event_list[i].is_enabled = false; // DISABLE Event
                    
                    // Ghi log và lưu NVS
                    Serial.printf("CLEANUP: Disabling one-time event '%s' (Index %d) as it finished.\n", event->label, i);
                    save_event_list();
                    
                    // KHÔNG cần tiếp tục logic bên dưới cho event này, vì nó đã bị disable.
                    // Nếu nó là event đang chạy, nó sẽ được xử lý ở bước 2.
                    continue; 
                }
            }

            // --- 2. TÌM ACTIVE EVENT (Event đang chạy hoặc vừa bắt đầu) ---
            
            if (is_event_active_on_selected_date(event, &current_date)) {
                
                bool is_start_time = (timeinfo.tm_hour == event->start_hour && timeinfo.tm_min == event->start_minute);
                
                // Logic is_within_time (Đoạn code phức tạp được giữ nguyên)
                bool is_within_time = false;
                if (event->start_hour < event->end_hour || 
                    (event->start_hour == event->end_hour && event->start_minute < event->end_minute)) 
                {
                    is_within_time = (timeinfo.tm_hour > event->start_hour || 
                                     (timeinfo.tm_hour == event->start_hour && timeinfo.tm_min >= event->start_minute)) &&
                                     (timeinfo.tm_hour < event->end_hour ||
                                     (timeinfo.tm_hour == event->end_hour && timeinfo.tm_min < event->end_minute));
                }

                if (is_within_time || is_start_time) { 
                    // Lưu Event Active (ưu tiên event có index cao nhất, do duyệt ngược)
                    next_active_event = event;
                    next_active_event_index = i;
                    // Không cần 'break', tiếp tục duyệt để hoàn thành Cleanup và tìm ưu tiên cao nhất.
                }
            }
        } // End for loop


        // -----------------------------------------------------------------
        // <<< BƯỚC 3: CẬP NHẬT BIẾN GLOBAL (Lấy Mutex 1 lần) >>>
        // -----------------------------------------------------------------

        if (xSemaphoreTake(g_event_mutex, portMAX_DELAY) == pdTRUE) {
            
            bool state_changed = false; // Cờ báo trạng thái g_is_event_running có thay đổi

            if (next_active_event != NULL) {
                // Event MỚI được tìm thấy hoặc Event ĐANG TIẾP TỤC chạy
                if (!g_is_event_running) {
                    Serial.printf("SCHEDULER: Starting event: %s\n", next_active_event->label);
                    state_changed = true;
                }
                // Sao chép trạng thái mới vào biến global
                memcpy(&g_current_active_event, next_active_event, sizeof(scheduler_event_t));
                g_is_event_running = true;
                g_active_scheduler_index = next_active_event_index; 

            } else {
                // Không có Event nào chạy
                if (g_is_event_running) {
                    Serial.println("SCHEDULER: Event ended. Resetting devices.");
                    state_changed = true;
                    // Reset g_current_active_event về trạng thái OFF/Level 0
                    memset(&g_current_active_event, 0, sizeof(scheduler_event_t));
                }
                g_is_event_running = false;
                g_active_scheduler_index = -1;
            }
            
            xSemaphoreGive(g_event_mutex);
            
            // --- BƯỚC 4: CẬP NHẬT UI (Gọi 1 lần an toàn) ---
            // Chỉ gọi update_screen2_ui() nếu có sự thay đổi lớn trong trạng thái Scheduler.
            if (state_changed) {
                 update_screen2_ui();
                 sync_all_attributes_to_server();
            }

        } else {
            Serial.println("ERROR: Could not take Mutex in GetTimeTask!");
        }

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}


extern "C" void app_main(void)
{
    arduino_setup();
    EEPROMDataInit();

    // Khởi tạo NVS và tải dữ liệu Event mới
    init_nvs_storage();
    load_event_list(); 
    
    // Khởi tạo EEPROM cũ (nếu vẫn dùng cho switch_data_array)
    EEPROMDataInit();
    

    setup_time();
    lvgl_setup();

    g_event_mutex = xSemaphoreCreateMutex();

    xTaskCreatePinnedToCore(
        networkTask,
        "Task Network",
        4096,
        NULL,
        4,
        NULL,
        0
    );
    xTaskCreatePinnedToCore(
        GetTimeTask,
        "Schedule Task",
        4096,
        NULL,
        2,
        NULL,
        1
    );
    // xTaskCreatePinnedToCore(
    //     SensorTask,
    //     "Task Sensor",
    //     10240,
    //     NULL,
    //     3,
    //     NULL,
    //     0
    // );
    // xTaskCreatePinnedToCore(
    //     SwitchDataTask,
    //     "Task Switch Data",
    //     8192,
    //     NULL,
    //     4,
    //     NULL,
    //     1
    // );
    // xTaskCreatePinnedToCore(
    //     I2CAddressTask,
    //     "Task I2C Address",
    //     4096,
    //     NULL,
    //     2,
    //     NULL,
    //     1
    // );
    xTaskCreatePinnedToCore(
        readCommandlineTask,
        "Task Read Command",
        4096,
        NULL,
        1,
        NULL,
        1
    );
    // xTaskCreatePinnedToCore(
    //     httpTask,
    //     "Task HTTP",
    //     8192,
    //     NULL,
    //     2,
    //     NULL,
    //     0
    // );
}

