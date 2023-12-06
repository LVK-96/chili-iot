#include <span>

#include <bme280.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/rcc.h>

#include "System.h"
#include "Temperature.h"

sensor_node_system::ErrorCode BME280TemperatureSensor::init()
{
    const int8_t res = bme280_init(&bme280);
    if (res != BME280_OK) {
        logger.error("Failed to initialize BME280!\n");
        return sensor_node_system::ErrorCode::TEMPERATURE_INIT_ERROR;
    }
    logger.info("BME280 initialized!\n");
    return sensor_node_system::ErrorCode::OK;
}

std::optional<double> BME280TemperatureSensor::read() const
{
    struct bme280_data read_data { };
    const int8_t res = bme280_get_sensor_data(BME280_TEMP, &read_data, &bme280);
    if (res != BME280_OK) {
        logger.error("Failed to read temperature!\n");
        return std::nullopt;
    }
    return read_data.temperature;
}

void BME280TemperatureSensor::write_reg(uint8_t addr, std::span<const uint8_t> data) const
{
    // Write BME280 register file @addr
    std::ignore = i2c.write(bme280_addr, addr);
    std::ignore = i2c.write(bme280_addr, data);
}

void BME280TemperatureSensor::read_reg(uint8_t addr, std::span<uint8_t> data) const
{
    // Read BME280 register file @addr
    std::ignore = i2c.write(bme280_addr, addr);
    std::ignore = i2c.read(bme280_addr, data);
}