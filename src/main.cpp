#include <memory>

#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include "FreeRTOSAdapter.h"
#include "RTOSTasks.h"
#include "System.h"

int main()
{
    // Setup the Bluepill board
    bluepill::setup();
    utils::logger.info("Board setup OK!\n");

    // FreeRTOS queue for the the temperature measurements
    // Temperature task writes to the queue, network task reads from the queue
    auto measurement_queue = xQueueCreate(measurement_queue_size, sizeof(double));

    // Temperature sensor, network interface & blinking LED
    auto temperature = std::make_unique<BME280TemperatureSensor>(&bluepill::peripherals::i2c1,
        BME280I2CBusAddr::SECONDARY); // The Waveshare BME280 module defaults to the secondary I2C address (0x77)
    auto rtos = std::make_unique<FreeRTOSAdapter>();
    auto network = std::make_unique<ESP8266Network>(
        &bluepill::peripherals::usart2, &bluepill::peripherals::esp_reset_pin, rtos.get());
    auto led = std::make_unique<GPIOLED>(&bluepill::peripherals::led_pin);

    // Build the argument structs for the tasks
    auto setup_args = std::make_unique<SetupTaskArgs>(nullptr, nullptr, nullptr, temperature.get(), network.get());
    auto temperature_args = std::make_unique<TemperatureTaskArgs>(measurement_queue, temperature.get());
    auto network_args = std::make_unique<NetworkTaskArgs>(measurement_queue, network.get());
    auto led_args = std::make_unique<LedTaskArgs>(led.get());

    // Regiter tasks to FreeRTOS
    xTaskHandle setup_task_handle = nullptr;
    xTaskHandle temperature_task_handle = nullptr;
    xTaskHandle network_task_handle = nullptr;
    xTaskCreate(setup_task, "SETUP", 256, setup_args.get(), configMAX_PRIORITIES - 1, &setup_task_handle);
    xTaskCreate(temperature_task, "TEMPERATURE", 256, temperature_args.get(), configMAX_PRIORITIES - 2,
        &temperature_task_handle);
    xTaskCreate(network_task, "NETWORK", 256, network_args.get(), configMAX_PRIORITIES - 3, &network_task_handle);
    xTaskCreate(led_task, "LED", configMINIMAL_STACK_SIZE, led_args.get(), configMAX_PRIORITIES - 4, nullptr);

    // Update the setup task args with the task handles
    setup_args->self = setup_task_handle;
    setup_args->temperature_task = temperature_task_handle;
    setup_args->network_task = network_task_handle;

    // Start the FreeRTOS scheduler
    utils::logger.info("Staring RTOS scheduler...\n");
    vTaskStartScheduler();
    utils::logger.error("RTOS scheduler returned! You don't have enough memory!\n");

    while (true) {
        ;
    }

    return -1; // This will never be reached
}