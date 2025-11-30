#pragma once

#include <cstdint>
#include <span>

#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/rcc.h>

#include "Peripheral.h"
#include "utils.h"

class Logger;

enum class BluePillI2C : unsigned int { _1 = I2C1, _2 = I2C2 };

class I2C final : public Peripheral {
public:
    constexpr I2C(BluePillI2C i2c_dev, rcc_periph_clken clken, rcc_periph_rst rst) noexcept
        : Peripheral(clken, rst)
        , i2c_dev(static_cast<unsigned int>(i2c_dev))
    {
    }
    void disable() const override;
    void enable() const override;
    void setup() const;
    [[nodiscard]] utils::ErrorCode read(uint8_t addr, std::span<uint8_t> data) const;
    [[nodiscard]] utils::ErrorCode write(uint8_t addr, std::span<const uint8_t> data) const;
    utils::ErrorCode read(uint8_t addr, uint8_t& data) const;
    [[nodiscard]] utils::ErrorCode write(uint8_t addr, uint8_t data) const;

private:
    unsigned int i2c_dev;
};