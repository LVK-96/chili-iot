#include "doctest.h"
#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include <cstring>
#include <string>

#include "test_events.h"
#include <algorithm>
#include <vector>

TEST_CASE("FreeRTOS records task create and queue operations")
{
    test_event_clear();

    // Call xTaskCreate to record a task
    xTaskCreate((TaskFunction_t) nullptr, "test_task", 128, nullptr, 5, nullptr);

    auto events = test_event_get_all();
    auto task_it = std::find_if(
        events.begin(), events.end(), [](const TestEvent& e) { return e.type == TestEventType::TaskCreate; });
    REQUIRE(task_it != events.end());

    // verify name: last N bytes are the name
    // data format: [stack(2), prio(1), namelen(1), name(namelen)]
    CHECK(task_it->data.size() > 4);
    uint8_t namelen = task_it->data[3];
    std::string name(reinterpret_cast<const char*>(task_it->data.data() + 4), namelen);
    CHECK(name == "test_task");

    uint16_t stack = task_it->data[0] | (task_it->data[1] << 8);
    CHECK(stack == 128);
    CHECK(task_it->data[2] == 5);

    // Create a queue and send an item
    QueueHandle_t q = xQueueCreate(4, 8);
    int value = 42;
    xQueueSend(q, &value, 0);

    events = test_event_get_all();
    auto send_it = std::find_if(
        events.begin(), events.end(), [](const TestEvent& e) { return e.type == TestEventType::QueueSend; });
    REQUIRE(send_it != events.end());
    // data format: [ticks(4), ...data]
    CHECK(send_it->data.size() >= 4);
    uint32_t ticks = send_it->data[0];
    CHECK(ticks == 0);

    // Receive from queue and verify value was received
    int recv_value = 0;
    xQueueReceive(q, &recv_value, 0);

    CHECK(recv_value == 42);

    events = test_event_get_all();
    auto recv_it = std::find_if(
        events.begin(), events.end(), [](const TestEvent& e) { return e.type == TestEventType::QueueReceive; });
    REQUIRE(recv_it != events.end());
    CHECK(recv_it->data.size() > 0);
}
