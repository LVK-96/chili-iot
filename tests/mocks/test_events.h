#pragma once

#include <cstdint>
#include <optional>
#include <vector>

enum class TestEventType : uint8_t {
    QueueSend = 1,
    QueueReceive = 2,
    UsartTx = 3,
    I2CTransfer = 4,
    DMAPeriphAddr = 5,
    DMAMemAddr = 6,
    DMANumber = 7,
    DMAEnable = 8,
    DMADisable = 9,
    TaskCreate = 10,
    UsartRxCount = 11,
};

struct TestEvent {
    TestEventType type;
    uint32_t id; // queue id or 0
    std::vector<uint8_t> data;
};

// Host/target implementations provide these
void test_event_record(TestEventType type, uint32_t id, const void* data, size_t len);
std::vector<TestEvent> test_event_get_all();
std::optional<TestEvent> test_event_get_last(TestEventType type);
void test_event_clear();

// Helpers to control mock state from tests in a portable way
void test_set_dma_cndtr(uint32_t value);
