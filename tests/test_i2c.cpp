#include "I2C.h"
#include "doctest.h"
#include "utils.h"

#include <array>
#include <span>

#include "test_events.h"
#include <algorithm>
#include <vector>

TEST_CASE("I2C basic operations")
{
    test_event_clear();

    I2C i2c1(BluePillI2C::_1, RCC_I2C1, RST_I2C1);

    // Basic lifecycle
    i2c1.setup();
    i2c1.enable();
    i2c1.disable();

    // Write/read span
    std::array<uint8_t, 3> write_buf = { 0x10, 0x20, 0x30 };
    auto write_res = i2c1.write(0x76, std::span<const uint8_t>(write_buf.data(), write_buf.size()));
    CHECK(write_res == utils::ErrorCode::OK);

    std::array<uint8_t, 3> read_buf = {};
    auto read_res = i2c1.read(0x76, std::span<uint8_t>(read_buf.data(), read_buf.size()));
    CHECK(read_res == utils::ErrorCode::OK);

    // Single byte variants
    uint8_t one = 0x55;
    CHECK(i2c1.write(0x76, one) == utils::ErrorCode::OK);
    uint8_t out = 0;
    CHECK(i2c1.read(0x76, out) == utils::ErrorCode::OK);

    // The i2c_transfer7 should have been invoked for each read/write call
    auto events = test_event_get_all();
    std::vector<TestEvent> i2c_events;
    std::copy_if(events.begin(), events.end(), std::back_inserter(i2c_events),
        [](const TestEvent& e) { return e.type == TestEventType::I2CTransfer; });

    CHECK(i2c_events.size() == 4);

    // Check last event details (single byte read)
    // Event data layout: [wlen, rlen, ...wdata]
    if (!i2c_events.empty()) {
        const auto& last = i2c_events.back();
        CHECK(last.id == 0x76);
        // rlen should be 1
        CHECK(last.data.size() >= 2);
        CHECK(last.data[1] == 1);
    }
}
