#include <span>
#include <string>

#include <bme280.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/rcc.h>

#include "System.h"
#include "Temperature.h"

BME280TemperatureSensor::BME280TemperatureSensor(const Logger& logger, const I2C& i2c)
    : logger(logger)
    , bme280({
        .chip_id = BME280_I2C_ADDR_SEC,
        .intf = BME280_I2C_INTF,
        .intf_ptr = (void*)&i2c,
        .intf_rslt = 0,
        .read = bme280_read,
        .write = bme280_write,
        .delay_us = bme280_delay_us,
        .calib_data = {}
    })
{
}

_system::ErrorCode BME280TemperatureSensor::init()
{
    int8_t res = bme280_init(&bme280);
    if (res != BME280_OK) {
        return _system::ErrorCode::TEMPERATURE_INIT_ERROR;
    }
    return _system::ErrorCode::OK;
}

std::optional<double> BME280TemperatureSensor::read() const
{
    struct bme280_data read_data;
    int8_t res = bme280_get_sensor_data(BME280_TEMP, &read_data, &bme280);
    if (res != BME280_OK) {
        return std::nullopt;
    }
    return read_data.temperature;
}