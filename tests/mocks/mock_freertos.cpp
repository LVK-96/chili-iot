#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

void vTaskDelay(const TickType_t xTicksToDelay) { (void)xTicksToDelay; }

void vTaskDelayUntil(TickType_t* const pxPreviousWakeTime, const TickType_t xTimeIncrement)
{
    (void)pxPreviousWakeTime;
    (void)xTimeIncrement;
}

TickType_t xTaskGetTickCount(void) { return 0; }

void vTaskSuspendAll(void) { }

void vTaskSuspend(TaskHandle_t xTaskToSuspend) { (void)xTaskToSuspend; }

BaseType_t xTaskCreate(TaskFunction_t pxTaskCode, const char* const pcName, const uint16_t usStackDepth,
    void* const pvParameters, UBaseType_t uxPriority, TaskHandle_t* const pxCreatedTask)
{
    (void)pxTaskCode;
    (void)pcName;
    (void)usStackDepth;
    (void)pvParameters;
    (void)uxPriority;
    (void)pxCreatedTask;
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
    (void)xQueue;
    (void)pvItemToQueue;
    (void)xTicksToWait;
    return pdTRUE;
}

BaseType_t xQueueSendToBack(QueueHandle_t xQueue, const void* const pvItemToQueue, TickType_t xTicksToWait)
{
    return xQueueSend(xQueue, pvItemToQueue, xTicksToWait);
}

BaseType_t xQueueReceive(QueueHandle_t xQueue, void* const pvBuffer, TickType_t xTicksToWait)
{
    (void)xQueue;
    (void)pvBuffer;
    (void)xTicksToWait;
    return pdTRUE;
}
