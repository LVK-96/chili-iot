#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/rcc.h>

#include "Peripheral.h"
#include "System.h"

class Logger;

enum class BluePillI2C : unsigned int { _1 = I2C1, _2 = I2C2 };

class I2C : public Peripheral {
public:
    constexpr I2C(BluePillI2C i2c_dev, rcc_periph_clken clken, rcc_periph_rst rst) noexcept
        : Peripheral(clken, rst)
        , i2c_dev(static_cast<unsigned int>(i2c_dev))
    {
    }
    void disable() override;
    void enable() override;
    void setup() const;
    sensor_node_system::ErrorCode read(uint8_t addr, std::span<uint8_t> data) const;
    sensor_node_system::ErrorCode write(uint8_t addr, std::span<const uint8_t> data) const;
    sensor_node_system::ErrorCode read(uint8_t addr, uint8_t& data) const;
    sensor_node_system::ErrorCode write(uint8_t addr, const uint8_t data) const;

private:
    const unsigned int i2c_dev;
    const uint32_t timeout = 0xffffff - 1;
};