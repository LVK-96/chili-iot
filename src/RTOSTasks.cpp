#include <cinttypes>

#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include "BlinkyLED.h"
#include "MQTTClient.h"
#include "Network.h"
#include "RTOSTasks.h"
#include "Temperature.h"
#include "utils.h"

#ifndef SERVER_IP
#define SERVER_IP "127.0.0.1"
#endif

#ifndef SERVER_PORT
#define SERVER_PORT 12345
#endif

static void setup_network(Network& network)
{
    utils::logger.info("Setting up network\n");
    while (network.init() != utils::ErrorCode::OK) {
        utils::logger.error("Failed to initialize network\n");
        ;
    }
}

static void setup_temperature(const TemperatureSensor& temperature)
{
    while (temperature.init() != utils::ErrorCode::OK) {
        ;
    }
}

void led_task(void* a)
{
    constexpr auto blink_delay = pdMS_TO_TICKS(1000);

    auto args = static_cast<LedTaskArgs*>(a);
    while (true) {
        // Blink the led
        auto wake_time = xTaskGetTickCount();
        args->led->toggle();
        xTaskDelayUntil(&wake_time, blink_delay);
    }
}

void temperature_task(void* a)
{
    constexpr auto measurement_delay = pdMS_TO_TICKS(10'000);

    // We should always have the notification already waiting as the setup task has higher priority
    if (pdTRUE != xTaskNotifyWait(0, 0, nullptr, portMAX_DELAY)) {
        utils::logger.error("Temperature task started before setup is done!\n");
        vTaskSuspendAll();
    }

    auto args = static_cast<TemperatureTaskArgs*>(a);
    while (true) {
        auto wake_time = xTaskGetTickCount();
        auto temperature_reading = args->temperature->read();
        // Read & send temperature
        if (temperature_reading) {
            xQueueSendToBack(args->measurement_queue, &(temperature_reading.value()), 0);
        }
        xTaskDelayUntil(&wake_time, measurement_delay);
    }
}

void network_task(void* a)
{
    // We should always have the notification already waiting as the setup task has higher priority
    if (pdTRUE != xTaskNotifyWait(0, 0, nullptr, portMAX_DELAY)) {
        utils::logger.error("Network task started before setup is done!\n");
        vTaskSuspendAll();
    }

    const auto args = static_cast<NetworkTaskArgs*>(a);

    MQTTClient mqtt_client(*(args->network));

    // Connect to MQTT broker
    // Retry indefinitely until connected
    while (mqtt_client.connect("chili-sensor", SERVER_IP, SERVER_PORT) != utils::ErrorCode::OK) {
        utils::logger.error("Failed to connect to MQTT broker, retrying...\n");
        bluepill::async_wait_ms(1000);
    }
    utils::logger.info("Connected to MQTT broker!\n");

    while (true) {
        double reading = 0;
        if (xQueueReceive(args->measurement_queue, &reading, portMAX_DELAY) != errQUEUE_EMPTY) {
            utils::logger.info("Sending reading...\n");

            std::span<const std::byte> payload_span(reinterpret_cast<const std::byte*>(&reading), sizeof(reading));

            if (mqtt_client.publish("sensors/temperature", payload_span) == utils::ErrorCode::OK) {
                utils::logger.info("Reading sent!\n");
            } else {
                utils::logger.error("Failed to publish reading!\n");
                // TODO: Try to reconnect?
            }
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
        printf("xTask: 0x%" PRIXPTR "\n", reinterpret_cast<uintptr_t>(xTask));
        printf("%s", pcTaskName);
        printf("\n");
        utils::nop(10'000);
    }
}
