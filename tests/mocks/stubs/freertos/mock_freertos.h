#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct MockTaskCreateCall {
    std::string name;
    uint16_t stack_depth;
    uint32_t priority;
};

struct MockQueueSendCall {
    uint32_t queue;
    std::vector<uint8_t> data;
    uint32_t ticks_to_wait;
};

struct MockQueueReceiveCall {
    uint32_t queue;
    uint32_t ticks_to_wait;
    std::vector<uint8_t> data;
};

extern std::vector<MockTaskCreateCall> mock_task_create_calls;
extern std::vector<MockQueueSendCall> mock_queue_send_calls;
extern std::vector<MockQueueReceiveCall> mock_queue_receive_calls;

void mock_freertos_reset();
