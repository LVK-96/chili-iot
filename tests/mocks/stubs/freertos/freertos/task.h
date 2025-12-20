#pragma once

#include <FreeRTOS.h>

typedef void (*TaskFunction_t)(void*);

BaseType_t xTaskCreate(TaskFunction_t pxTaskCode,
    const char* const pcName, /*lint !e971 Unqualified char types are allowed for strings and single characters only. */
    const uint16_t usStackDepth, void* const pvParameters, UBaseType_t uxPriority, TaskHandle_t* const pxCreatedTask);

void vTaskStartScheduler(void);

typedef enum { eNoAction = 0, eSetBits, eIncrement, eSetValueWithOverwrite, eSetValueWithoutOverwrite } eNotifyAction;

BaseType_t xTaskNotify(TaskHandle_t xTaskToNotify, uint32_t ulValue, eNotifyAction eAction);
BaseType_t xTaskNotifyWait(uint32_t ulBitsToClearOnEntry, uint32_t ulBitsToClearOnExit, uint32_t* pulNotificationValue,
    TickType_t xTicksToWait);

TaskHandle_t xTaskGetCurrentTaskHandle(void);

#define xTaskDelayUntil vTaskDelayUntil
