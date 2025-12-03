#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "mock_freertos.h"
#include <cstdint>
#include <cstring>
#include <deque>
#include <unordered_map>
#include <vector>

// Simple counters for tests to inspect
// Detailed logs
std::vector<MockTaskCreateCall> mock_task_create_calls;
std::vector<MockQueueSendCall> mock_queue_send_calls;
std::vector<MockQueueReceiveCall> mock_queue_receive_calls;

// Queue storage: map from queue handle to a deque of data blocks
static std::unordered_map<uintptr_t, std::deque<std::vector<uint8_t>>> mock_queue_storage;

void vTaskDelay(const TickType_t xTicksToDelay) { (void)xTicksToDelay; }

void vTaskDelayUntil(TickType_t* const pxPreviousWakeTime, const TickType_t xTimeIncrement)
{
    (void)pxPreviousWakeTime;
    (void)xTimeIncrement;
}

TickType_t xTaskGetTickCount(void)
{
    static TickType_t mock_tick_count = 0;
    // Advance the tick count on each call so busy-wait loops make progress in host tests
    return ++mock_tick_count;
}

void vTaskSuspendAll(void) { }

void vTaskSuspend(TaskHandle_t xTaskToSuspend) { (void)xTaskToSuspend; }

BaseType_t xTaskCreate(TaskFunction_t pxTaskCode, const char* const pcName, const uint16_t usStackDepth,
    void* const pvParameters, UBaseType_t uxPriority, TaskHandle_t* const pxCreatedTask)
{
    (void)pxTaskCode;
    (void)pvParameters;
    (void)pxCreatedTask;
    MockTaskCreateCall call;
    if (pcName) {
        call.name = pcName;
    }
    call.stack_depth = usStackDepth;
    call.priority = uxPriority;
    mock_task_create_calls.push_back(std::move(call));
    return pdTRUE;
}

BaseType_t xTaskCreate_record(TaskFunction_t pxTaskCode, const char* const pcName, const uint16_t usStackDepth,
    void* const pvParameters, UBaseType_t uxPriority, TaskHandle_t* const pxCreatedTask)
{
    (void)pxTaskCode;
    (void)pvParameters;
    (void)pxCreatedTask;
    MockTaskCreateCall call;
    if (pcName) {
        call.name = pcName;
    }
    call.stack_depth = usStackDepth;
    call.priority = uxPriority;
    mock_task_create_calls.push_back(std::move(call));
    return pdTRUE;
}

void vTaskStartScheduler(void) { }

BaseType_t xTaskNotify(TaskHandle_t xTaskToNotify, uint32_t ulValue, eNotifyAction eAction)
{
    (void)xTaskToNotify;
    (void)ulValue;
    (void)eAction;
    return pdPASS;
}

BaseType_t xTaskNotifyWait(uint32_t ulBitsToClearOnEntry, uint32_t ulBitsToClearOnExit, uint32_t* pulNotificationValue,
    TickType_t xTicksToWait)
{
    (void)ulBitsToClearOnEntry;
    (void)ulBitsToClearOnExit;
    (void)pulNotificationValue;
    (void)xTicksToWait;
    return pdPASS;
}

QueueHandle_t xQueueCreate(UBaseType_t uxQueueLength, UBaseType_t uxItemSize)
{
    (void)uxQueueLength;
    (void)uxItemSize;
    return (QueueHandle_t)1;
}

BaseType_t xQueueSend(QueueHandle_t xQueue, const void* const pvItemToQueue, TickType_t xTicksToWait)
{
    uintptr_t queue_key = reinterpret_cast<uintptr_t>(xQueue);
    MockQueueSendCall call;
    call.queue = static_cast<uint32_t>(queue_key);
    call.ticks_to_wait = xTicksToWait;
    if (pvItemToQueue) {
        const uint8_t* data = static_cast<const uint8_t*>(pvItemToQueue);
        size_t to_copy = 16; // best-effort copy; tests should keep this reasonable
        call.data.assign(data, data + to_copy);
        // Store data in the queue
        mock_queue_storage[queue_key].push_back(call.data);
    }
    mock_queue_send_calls.push_back(std::move(call));
    return pdTRUE;
}

BaseType_t xQueueSendToBack(QueueHandle_t xQueue, const void* const pvItemToQueue, TickType_t xTicksToWait)
{
    return xQueueSend(xQueue, pvItemToQueue, xTicksToWait);
}

BaseType_t xQueueReceive(QueueHandle_t xQueue, void* const pvBuffer, TickType_t xTicksToWait)
{
    uintptr_t queue_key = reinterpret_cast<uintptr_t>(xQueue);
    MockQueueReceiveCall call;
    call.queue = static_cast<uint32_t>(queue_key);
    call.ticks_to_wait = xTicksToWait;

    // Try to retrieve data from the queue storage
    if (mock_queue_storage.count(queue_key) && !mock_queue_storage[queue_key].empty()) {
        auto data = std::move(mock_queue_storage[queue_key].front());
        mock_queue_storage[queue_key].pop_front();
        call.data = data;
        if (pvBuffer) {
            std::memcpy(pvBuffer, data.data(), data.size());
        }
    } else {
        // Queue is empty; fill buffer with zeroes
        call.data.clear();
        if (pvBuffer) {
            std::memset(pvBuffer, 0, 16);
        }
    }
    mock_queue_receive_calls.push_back(std::move(call));
    return pdTRUE;
}

void mock_freertos_reset()
{
    mock_task_create_calls.clear();
    mock_queue_send_calls.clear();
    mock_queue_receive_calls.clear();
    mock_queue_storage.clear();
}
