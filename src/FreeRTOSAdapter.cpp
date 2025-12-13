#include "FreeRTOSAdapter.h"

#include <FreeRTOS.h>
#include <task.h>

void FreeRTOSAdapter::delay(uint32_t ms) const { vTaskDelay(pdMS_TO_TICKS(ms)); }

uint32_t FreeRTOSAdapter::get_tick_count() const { return xTaskGetTickCount(); }

bool FreeRTOSAdapter::task_notify_wait(uint32_t timeout_ms) const
{
    return xTaskNotifyWait(0, 0, nullptr, pdMS_TO_TICKS(timeout_ms)) == pdTRUE;
}

IRTOS::TaskHandle FreeRTOSAdapter::get_current_task_handle() const { return xTaskGetCurrentTaskHandle(); }
