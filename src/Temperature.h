#pragma once

#include <optional>

#include <bme280.h>
#include <bme280_defs.h>
#include <libopencm3/stm32/i2c.h>

#include "I2C.h"
#include "Logger.h"
#include "System.h"

class TemperatureSensor {
public:
    [[nodiscard]] virtual std::optional<double> read() const = 0;
};

enum class BME280I2CBusAddr : uint8_t { PRIMARY = BME280_I2C_ADDR_PRIM, SECONDARY = BME280_I2C_ADDR_SEC };

class BME280TemperatureSensor final : public TemperatureSensor {
public:
    constexpr BME280TemperatureSensor(const Logger* logger, const I2C* i2c, BME280I2CBusAddr bme280_addr) noexcept
        : logger(logger)
        , i2c(i2c)
        , bme280_addr(static_cast<uint8_t>(bme280_addr))
        , bme280({ .chip_id = BME280_I2C_ADDR_SEC,
              .intf = BME280_I2C_INTF,
              .intf_ptr = static_cast<void*>(this),
              .intf_rslt = 0,
              .read = bme280_read,
              .write = bme280_write,
              .delay_us = bme280_delay_us,
              .calib_data = {} })
    {
    }

    sensor_node_system::ErrorCode init();
    std::optional<double> read() const override;
    void write_reg(uint8_t addr, std::span<const uint8_t> data) const;
    void read_reg(uint8_t addr, std::span<uint8_t> data) const;

private:
    const Logger* logger;
    const I2C* i2c;
    uint8_t bme280_addr;
    mutable struct bme280_dev bme280;

    // For the Bosch BME280 driver
    static BME280_INTF_RET_TYPE bme280_read(uint8_t reg_addr, uint8_t* reg_data, uint32_t len, void* intf_ptr)
    {
        auto* bme280_sensor = static_cast<BME280TemperatureSensor*>(intf_ptr);
        auto sp = std::span<uint8_t>(reg_data, len);
        bme280_sensor->read_reg(reg_addr, sp);
        return BME280_INTF_RET_SUCCESS;
    }

    static BME280_INTF_RET_TYPE bme280_write(uint8_t reg_addr, const uint8_t* reg_data, uint32_t len, void* intf_ptr)
    {
        auto* bme280_sensor = static_cast<BME280TemperatureSensor*>(intf_ptr);
        auto sp = std::span<const uint8_t>(reg_data, len);
        bme280_sensor->write_reg(reg_addr, sp);
        return BME280_INTF_RET_SUCCESS;
    }

    static void bme280_delay_us(uint32_t period, [[maybe_unused]] void* intf_ptr)
    {
        // This will only be called with period == 2000 from the BOSCH driver
        // so this is OK as we don't need a more accurate sleep, but not ideal
        sensor_node_system::sleep_ms(period / 1000);
    }
};