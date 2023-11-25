#include <span>
#include <string>

#include <bme280.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/rcc.h>

#include "System.h"
#include "Temperature.h"

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