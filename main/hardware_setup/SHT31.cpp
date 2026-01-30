//
//    FILE: SHT31.cpp
//  AUTHOR: Rob Tillaart
// VERSION: 0.5.1
//    DATE: 2019-02-08
// PURPOSE: Arduino library for the SHT31 temperature and humidity sensor
//          https://www.adafruit.com/product/2857
//     URL: https://github.com/RobTillaart/SHT31


#include "SHT31.h"
#include <string.h>


//  SUPPORTED COMMANDS - single shot mode only
#define SHT31_READ_STATUS       0xF32D
#define SHT31_CLEAR_STATUS      0x3041

#define SHT31_SOFT_RESET        0x30A2
#define SHT31_HARD_RESET        0x0006

#define SHT31_MEASUREMENT_FAST  0x2416     //  page 10 datasheet
#define SHT31_MEASUREMENT_SLOW  0x2400     //  no clock stretching

#define SHT31_HEAT_ON           0x306D
#define SHT31_HEAT_OFF          0x3066
#define SHT31_HEATER_TIMEOUT    180000UL   //  milliseconds

#define I2C_TIMEOUT_MS 1000

SHT31::SHT31(uint8_t address, i2c_port_t port)
{
    _i2c_port = port;
    _address = address;
    _lastRead = 0;
    _rawTemperature = 0;
    _rawHumidity = 0;
    _heatTimeout = 0;
    _heaterStart = 0;
    _heaterStop = 0;
    _heaterOn = false;
    _error = SHT31_OK;
}

bool SHT31::begin()
{
    if ((_address != 0x44) && (_address != 0x45)) return false;
    return reset();
}

bool SHT31::isConnected()
{
    uint8_t dummy = 0;
    esp_err_t err = i2c_master_write_to_device(_i2c_port, _address, &dummy, 0, pdMS_TO_TICKS(I2C_TIMEOUT_MS));
    if (err != ESP_OK)
    {
        _error = SHT31_ERR_NOT_CONNECT;
        return false;
    }
    return true;
}

bool SHT31::read(bool fast)
{
    if (!writeCmd(fast ? SHT31_MEASUREMENT_FAST : SHT31_MEASUREMENT_SLOW)) return false;
    vTaskDelay(pdMS_TO_TICKS(fast ? 4 : 15));
    return readData(fast);
}

uint16_t SHT31::readStatus()
{
    uint8_t status[3];
    if (!writeCmd(SHT31_READ_STATUS)) return 0xFFFF;

    esp_err_t err = i2c_master_read_from_device(_i2c_port, _address, status, 3, pdMS_TO_TICKS(I2C_TIMEOUT_MS));
    if (err != ESP_OK || status[2] != crc8(status, 2))
    {
        _error = SHT31_ERR_CRC_STATUS;
        return 0xFFFF;
    }

    return (uint16_t)(status[0] << 8 | status[1]);
}

bool SHT31::clearStatus()
{
    return writeCmd(SHT31_CLEAR_STATUS);
}

bool SHT31::reset(bool hard)
{
    bool b = writeCmd(hard ? SHT31_HARD_RESET : SHT31_SOFT_RESET);
    vTaskDelay(pdMS_TO_TICKS(1));
    return b;
}

void SHT31::setHeatTimeout(uint8_t seconds)
{
    _heatTimeout = (seconds > 180) ? 180 : seconds;
}

bool SHT31::heatOn()
{
    if (isHeaterOn()) return true;

    if ((_heaterStop > 0) && (esp_timer_get_time() / 1000 - _heaterStop < SHT31_HEATER_TIMEOUT))
    {
        _error = SHT31_ERR_HEATER_COOLDOWN;
        return false;
    }

    if (!writeCmd(SHT31_HEAT_ON))
    {
        _error = SHT31_ERR_HEATER_ON;
        return false;
    }

    _heaterStart = esp_timer_get_time() / 1000;
    _heaterOn = true;
    return true;
}

bool SHT31::heatOff()
{
    if (!writeCmd(SHT31_HEAT_OFF))
    {
        _error = SHT31_ERR_HEATER_OFF;
        return false;
    }

    _heaterStop = esp_timer_get_time() / 1000;
    _heaterOn = false;
    return true;
}

bool SHT31::isHeaterOn()
{
    if (!_heaterOn) return false;

    if ((esp_timer_get_time() / 1000) - _heaterStart < (_heatTimeout * 1000UL))
    {
        return true;
    }

    heatOff();
    return false;
}

bool SHT31::requestData()
{
    if (!writeCmd(SHT31_MEASUREMENT_SLOW)) return false;
    _lastRequest = esp_timer_get_time() / 1000;
    return true;
}

bool SHT31::dataReady()
{
    return ((esp_timer_get_time() / 1000 - _lastRequest) > 15);
}

bool SHT31::readData(bool fast)
{
    uint8_t buffer[6];
    esp_err_t err = i2c_master_read_from_device(_i2c_port, _address, buffer, 6, pdMS_TO_TICKS(I2C_TIMEOUT_MS));
    if (err != ESP_OK) return false;

    if (!fast)
    {
        if (buffer[2] != crc8(buffer, 2))
        {
            _error = SHT31_ERR_CRC_TEMP;
            return false;
        }
        if (buffer[5] != crc8(buffer + 3, 2))
        {
            _error = SHT31_ERR_CRC_HUM;
            return false;
        }
    }

    _rawTemperature = (buffer[0] << 8) | buffer[1];
    _rawHumidity = (buffer[3] << 8) | buffer[4];
    _lastRead = esp_timer_get_time() / 1000;

    return true;
}

int SHT31::getError()
{
    int rv = _error;
    _error = SHT31_OK;
    return rv;
}

uint8_t SHT31::crc8(const uint8_t *data, uint8_t len)
{
    const uint8_t POLY = 0x31;
    uint8_t crc = 0xFF;

    for (uint8_t j = len; j; --j)
    {
        crc ^= *data++;
        for (uint8_t i = 8; i; --i)
        {
            crc = (crc & 0x80) ? (crc << 1) ^ POLY : (crc << 1);
        }
    }
    return crc;
}

bool SHT31::writeCmd(uint16_t cmd)
{
    uint8_t data[2] = { (uint8_t)(cmd >> 8), (uint8_t)(cmd & 0xFF) };
    esp_err_t err = i2c_master_write_to_device(_i2c_port, _address, data, 2, pdMS_TO_TICKS(I2C_TIMEOUT_MS));
    if (err != ESP_OK)
    {
        _error = SHT31_ERR_WRITECMD;
        return false;
    }
    return true;
}
