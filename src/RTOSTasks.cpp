#include <cstdio>
#include <memory>
#include <utility>

#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include "BlinkyLED.h"
#include "Network.h"
#include "RTOSTasks.h"
#include "System.h"
#include "Temperature.h"
#include "utils.h"

static void setup_network(const Network& network)
{
    while (network.init() != utils::ErrorCode::OK) {
        ;
    }
}

static void setup_temperature(const TemperatureSensor& temperature)
{
    while (temperature.init() != utils::ErrorCode::OK) {
        ;
    }
}

void led_task([[maybe_unused]] void* a)
{
    constexpr auto blink_delay = pdMS_TO_TICKS(1000);

    auto args = static_cast<LedTaskArgs*>(a);
    auto wake_time = xTaskGetTickCount();
    while (true) {
        // Blink the led
        args->led->toggle();
        vTaskDelayUntil(&wake_time, blink_delay);
    }
}

void temperature_task(void* a)
{
    constexpr auto measurement_delay = pdMS_TO_TICKS(2000);

    // We should always have the notification already waiting as the setup task has higher priority
    if (pdTRUE != xTaskNotifyWait(0, 0, nullptr, 0)) {
        utils::logger.error("Temperature task started before setup is done!\n");
        vTaskSuspendAll();
    }

    auto args = static_cast<TemperatureTaskArgs*>(a);
    auto wake_time = xTaskGetTickCount();
    auto last_temperature = args->temperature->read();
    while (true) {
        // Read & send temperature
        if (last_temperature) {
            xQueueSendToBack(args->measurement_queue, &(last_temperature.value()), 0);
        }
        last_temperature = args->temperature->read();
        vTaskDelayUntil(&wake_time, measurement_delay);
    }
}

void network_task(void* a)
{
    // We should always have the notification already waiting as the setup task has higher priority
    if (pdTRUE != xTaskNotifyWait(0, 0, nullptr, 0)) {
        utils::logger.error("Network task started before setup is done!\n");
        vTaskSuspendAll();
    }

    const auto args = static_cast<NetworkTaskArgs*>(a);

    while (true) {
        double reading = 0;
        if (xQueueReceive(args->measurement_queue, &reading, portMAX_DELAY) != errQUEUE_EMPTY) {
            args->network->publish_measurement(reading);
        }
    }
}

void setup_task(void* a)
{
    const auto args = static_cast<SetupTaskArgs*>(a);
    while (true) {
        setup_temperature(*(args->temperature));
        xTaskNotify(args->temperature_task, 0, eNoAction);
        setup_network(*(args->network));
        xTaskNotify(args->network_task, 0, eNoAction);
        vTaskSuspend(args->self);
    }
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char* pcTaskName)
{
    while (true) {
        utils::logger.error("STACK OVERFLOW!\n");
        printf("xTask: 0x%lx\n", reinterpret_cast<uint32_t>(xTask));
        printf(pcTaskName);
        printf("\n");
        utils::nop(10'000);
    }
}