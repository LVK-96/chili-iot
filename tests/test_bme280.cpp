#include "BME280TemperatureSensor.h"
#include "I2C.h"
#include "doctest.h"
#include "test_events.h"

#include <array>
#include <optional>

TEST_CASE("BME280TemperatureSensor read/write register via I2C")
{
    I2C i2c1(BluePillI2C::_1, RCC_I2C1, RST_I2C1);
    BME280TemperatureSensor sensor(&i2c1, BME280I2CBusAddr::PRIMARY);

    // Exercise write_reg/read_reg which use the I2C wrapper. The underlying
    // libopencm3 calls are simulated, so these should be no-ops and not crash.
    std::array<uint8_t, 2> w = { 0xAA, 0xBB };
    sensor.write_reg(0x10, w);

    std::array<uint8_t, 2> r = {};
    sensor.read_reg(0x10, r);

    // We don't expect any meaningful data from the backend, just that
    // the calls complete without error.
    CHECK(true);
}