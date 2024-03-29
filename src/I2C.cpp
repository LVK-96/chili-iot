#include <libopencm3/stm32/i2c.h>

#include "I2C.h"
#include "Logger.h"

void I2C::setup() const
{
    I2C_CR1(i2c_dev) &= ~I2C_CR1_STOP; // Clear stop
    i2c_set_speed(i2c_dev, i2c_speed_sm_100k, rcc_apb1_frequency / 1e6);
    i2c_set_own_7bit_slave_address(i2c_dev, 0xff); // Why is this necessary???
}

void I2C::enable() const { i2c_peripheral_enable(i2c_dev); }

void I2C::disable() const { i2c_peripheral_disable(i2c_dev); }

utils::ErrorCode I2C::write(uint8_t addr, std::span<const uint8_t> data) const
{
    i2c_transfer7(i2c_dev, addr, data.data(), data.size(), nullptr, 0);
    return utils::ErrorCode::OK;
}

utils::ErrorCode I2C::read(uint8_t addr, std::span<uint8_t> data) const
{
    i2c_transfer7(i2c_dev, addr, nullptr, 0, data.data(), data.size());
    return utils::ErrorCode::OK;
}

utils::ErrorCode I2C::write(uint8_t addr, uint8_t data) const
{
    return write(addr, std::span<const uint8_t, 1>(&data, 1));
}

utils::ErrorCode I2C::read(uint8_t addr, uint8_t& data) const { return read(addr, std::span<uint8_t, 1>(&data, 1)); }