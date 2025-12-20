#include <cstdio>
#include <span>
#include <tuple>

#include <bme280.h>

#include "BME280TemperatureSensor.h"
#include "Logger.h"
#include "utils.h"

utils::ErrorCode BME280TemperatureSensor::init() const
{
    utils::logger.info("Initializing BME280...\n");
    const int8_t res = bme280_init(&bme280);
    if (res != BME280_OK) {
        utils::logger.error("Failed to initialize BME280!\n");
        return utils::ErrorCode::TEMPERATURE_INIT_ERROR;
    }
    utils::logger.info("BME280 initialized!\n");
    return utils::ErrorCode::OK;
}

std::optional<double> BME280TemperatureSensor::read() const
{
    utils::logger.info("Reading BME280...\n");
    struct bme280_data read_data {};
    const int8_t res = bme280_get_sensor_data(BME280_TEMP, &read_data, &bme280);
    if (res != BME280_OK) {
        utils::logger.error("Failed to read temperature!\n");
        return std::nullopt;
    }
    utils::logger.info("BME280 read, temperatue: %d!\n", static_cast<int>(read_data.temperature));
    return read_data.temperature;
}

void BME280TemperatureSensor::write_reg(const uint8_t addr, std::span<const uint8_t> data) const
{
    // Write BME280 register file @addr
    std::ignore = i2c->write(bme280_addr, std::span<const uint8_t>(&addr, 1));
    std::ignore = i2c->write(bme280_addr, data);
}

void BME280TemperatureSensor::read_reg(const uint8_t addr, std::span<uint8_t> data) const
{
    // Read BME280 register file @addr
    std::ignore = i2c->write(bme280_addr, std::span<const uint8_t>(&addr, 1));
    std::ignore = i2c->read(bme280_addr, data);
}