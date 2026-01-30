#include "esp_err.h"
#include "esp_log.h"
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define I2C_MASTER_NUM       I2C_NUM_0  // I2C port number
#define I2C_FREQ_HZ          400000  // I2C frequency (400kHz)

#define PCF8574_ADDR         0x20    // PCF8574 I2C address (can be between 0x20 and 0x27)

void PCF8574_WritePin(uint8_t pin, bool level);
bool PCF8574_ReadPin(uint8_t pin);