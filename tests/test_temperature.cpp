#include "I2C.h"
#include "Temperature.h"
#include "doctest.h"
#include "mock_libopencm3.h"

#include <array>
#include <optional>

TEST_CASE("BME280TemperatureSensor read/write register via mocked I2C")
{
    I2C i2c1(BluePillI2C::_1, RCC_I2C1, RST_I2C1);
    BME280TemperatureSensor sensor(&i2c1, BME280I2CBusAddr::PRIMARY);

    // Exercise write_reg/read_reg which use the I2C wrapper. The underlying
    // libopencm3 calls are mocked, so these should be no-ops and not crash.
    std::array<uint8_t, 2> w = { 0xAA, 0xBB };
    sensor.write_reg(0x10, w);

    std::array<uint8_t, 2> r = {};
    sensor.read_reg(0x10, r);

    // We don't expect any meaningful data from the mocked backend, just that
    // the calls complete without error.
    CHECK(true);
}

TEST_CASE("BME280TemperatureSensor init and read")
{
    mock_libopencm3_reset();

    I2C i2c1(BluePillI2C::_1, RCC_I2C1, RST_I2C1);
    BME280TemperatureSensor sensor(&i2c1, BME280I2CBusAddr::PRIMARY);

    // Initialization uses the Bosch driver which will perform I2C operations
    auto init_res = sensor.init();
    CHECK(init_res == utils::ErrorCode::OK);

    auto reading = sensor.read();
    CHECK(reading.has_value());

    // Verify mock returns compensated temperature within acceptable error of 21.0°C
    // The mock is configured to return ~20.9999°C, so allow ±0.1°C tolerance
    double temp_value = reading.value();
    CHECK(temp_value > 20.9);
    CHECK(temp_value < 21.1);
}
