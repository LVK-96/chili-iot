#pragma once

#include <cstdint>

#define pdTRUE 1
#define pdFALSE 0

#define configMAX_PRIORITIES 5
#define configMINIMAL_STACK_SIZE 128

typedef uint32_t TickType_t;
typedef uint32_t BaseType_t;
typedef uint32_t UBaseType_t;
typedef void* TaskHandle_t;
typedef TaskHandle_t xTaskHandle;
typedef void* QueueHandle_t;
typedef void* SemaphoreHandle_t;

#define portTICK_PERIOD_MS 1
#define portTICK_RATE_MS portTICK_PERIOD_MS
#define portMAX_DELAY 0xFFFFFFFF

#define pdMS_TO_TICKS(x) ((x) / portTICK_PERIOD_MS)

#define pdPASS 1
#define pdFAIL 0
#define errQUEUE_EMPTY 0

void vTaskDelay(const TickType_t xTicksToDelay);
void vTaskDelayUntil(TickType_t* const pxPreviousWakeTime, const TickType_t xTimeIncrement);
TickType_t xTaskGetTickCount(void);
void vTaskSuspendAll(void);
void vTaskSuspend(TaskHandle_t xTaskToSuspend);
