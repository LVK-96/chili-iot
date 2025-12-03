#include "doctest.h"
#include "mock_freertos.h"
#include "mock_libopencm3.h"
#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include <cstring>

TEST_CASE("FreeRTOS mock records task create and queue operations")
{
    mock_freertos_reset();
    mock_libopencm3_reset();

    // Call xTaskCreate to record a task
    xTaskCreate((TaskFunction_t) nullptr, "test_task", 128, nullptr, 5, nullptr);
    REQUIRE(mock_task_create_calls.size() == 1);
    CHECK(mock_task_create_calls[0].name == "test_task");
    CHECK(mock_task_create_calls[0].stack_depth == 128);
    CHECK(mock_task_create_calls[0].priority == 5);

    // Create a queue and send an item
    QueueHandle_t q = xQueueCreate(4, 8);
    int value = 42;
    xQueueSend(q, &value, 0);
    REQUIRE(mock_queue_send_calls.size() == 1);
    CHECK(mock_queue_send_calls[0].ticks_to_wait == 0);

    // Receive from queue and verify value was received
    int recv_value = 0;
    xQueueReceive(q, &recv_value, 0);
    REQUIRE(mock_queue_receive_calls.size() == 1);
    CHECK(recv_value == 42);
    CHECK(mock_queue_receive_calls[0].data.size() > 0);
}
