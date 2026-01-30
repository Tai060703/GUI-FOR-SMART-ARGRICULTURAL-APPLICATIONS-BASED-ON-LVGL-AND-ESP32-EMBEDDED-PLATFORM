#include "PCF8574.h"

static uint8_t pcf8574_output_state = 0xFF;


static const char *TAG = "PCF8574";

void PCF8574_Write(uint8_t data) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (PCF8574_ADDR << 1) | I2C_MASTER_WRITE, true); // Write command
    i2c_master_write_byte(cmd, data, true); // Send data byte
    i2c_master_stop(cmd);

    esp_err_t err = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000)); // Timeout in ms
    i2c_cmd_link_delete(cmd);

    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Data written: 0x%02X", data);
    } else {
        ESP_LOGE(TAG, "Error writing data: %s", esp_err_to_name(err));
    }
}

uint8_t PCF8574_Read() {
    uint8_t data = 0;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (PCF8574_ADDR << 1) | I2C_MASTER_READ, true); // Read command
    i2c_master_read_byte(cmd, &data, I2C_MASTER_LAST_NACK); // Read data byte
    i2c_master_stop(cmd);

    esp_err_t err = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000)); // Timeout in ms
    i2c_cmd_link_delete(cmd);

    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Data read: 0x%02X", data);
    } else {
        ESP_LOGE(TAG, "Error reading data: %s", esp_err_to_name(err));
    }

    return data;
}

void PCF8574_WritePin(uint8_t pin, bool level) {
    if (pin > 7) return; // Invalid pin

    if (level) {
        pcf8574_output_state |= (1 << pin); // Set bit
    } else {
        pcf8574_output_state &= ~(1 << pin); // Clear bit
    }

    PCF8574_Write(pcf8574_output_state);
}

bool PCF8574_ReadPin(uint8_t pin) {
    if (pin > 7) return false;

    uint8_t data = PCF8574_Read();
    return (data & (1 << pin)) != 0;
}