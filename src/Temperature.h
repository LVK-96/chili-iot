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
    virtual std::optional<double> read() const = 0;
};

class BME280TemperatureSensor : public TemperatureSensor {
public:
    constexpr BME280TemperatureSensor(const Logger& logger, const I2C& i2c) noexcept
        : logger(logger)
        , bme280({ .chip_id = BME280_I2C_ADDR_SEC,
              .intf = BME280_I2C_INTF,
              .intf_ptr = (void*)&i2c,
              .intf_rslt = 0,
              .read = bme280_read,
              .write = bme280_write,
              .delay_us = bme280_delay_us,
              .calib_data = {} })
    {
    }

    _system::ErrorCode init();
    std::optional<double> read() const override;

private:
    const Logger& logger;
    mutable struct bme280_dev bme280;

    // For the Bosch BME280 driver
    static BME280_INTF_RET_TYPE bme280_read(uint8_t reg_addr, uint8_t* reg_data, uint32_t len, void* intf_ptr)
    {
        constexpr uint8_t BME280_I2C_BUS_ADDR = BME280_I2C_ADDR_SEC;
        const I2C* i2c = (I2C*)intf_ptr;
        i2c->write(BME280_I2C_BUS_ADDR, reg_addr);
        auto sp = std::span<uint8_t>(reg_data, len);
        i2c->read(BME280_I2C_BUS_ADDR, sp);
        return BME280_INTF_RET_SUCCESS;
    }

    static BME280_INTF_RET_TYPE bme280_write(uint8_t reg_addr, const uint8_t* reg_data, uint32_t len, void* intf_ptr)
    {
        constexpr uint8_t BME280_I2C_BUS_ADDR = BME280_I2C_ADDR_SEC;
        I2C* i2c = (I2C*)intf_ptr;
        i2c->write(BME280_I2C_BUS_ADDR, reg_addr);
        auto sp = std::span<const uint8_t>(reg_data, len);
        i2c->write(BME280_I2C_BUS_ADDR, sp);
        return BME280_INTF_RET_SUCCESS;
    }

    static void bme280_delay_us(uint32_t period, [[maybe_unused]] void* intf_ptr)
    {
        _system::sleep_us(period);
    }
};