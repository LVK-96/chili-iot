#include <memory>

#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include "Factories.h"
#include "FreeRTOSAdapter.h"
#include "I2C.h"
#include "Logger.h"
#include "RTOSTasks.h"
#include "System.h"
#include "interfaces/ILED.h"
#include "interfaces/INetwork.h"
#include "interfaces/IRTOS.h"
#include "interfaces/ITemperatureSensor.h"

#ifdef SEMIHOSTING_ENV
extern "C" void initialise_monitor_handles(void);
#endif

// Use static objects to ensure they live forever and are not clobbered by stack reuse
static std::unique_ptr<ITemperatureSensor> temperature;
static std::unique_ptr<IRTOS> rtos_adapter;
static std::unique_ptr<INetwork> network;
static std::unique_ptr<ILED> led;

static std::unique_ptr<SetupTaskArgs> setup_args;
static std::unique_ptr<TemperatureTaskArgs> temperature_args;
static std::unique_ptr<NetworkTaskArgs> network_args;
static std::unique_ptr<LedTaskArgs> led_args;

static QueueHandle_t measurement_queue = nullptr;

int main()
{
#ifdef SEMIHOSTING_ENV
    initialise_monitor_handles();
    printf("===SEMIHOSTING_ENV===\n");
#endif // SEMIHOSTING_ENV

    // Setup the Bluepill board
    bluepill::setup();
    utils::logger.info("Board setup OK!\n");

    // FreeRTOS queue for the the temperature measurements
    // Temperature task writes to the queue, network task reads from the queue
    measurement_queue = xQueueCreate(measurement_queue_size, sizeof(double));

    // Initialize globals
    temperature = createTemperatureSensor();
    rtos_adapter = createRTOS();
    network = createNetwork(rtos_adapter.get());
    led = createLED();

    // Build the argument structs for the tasks
    setup_args = std::make_unique<SetupTaskArgs>(nullptr, nullptr, nullptr, temperature.get(), network.get());
    temperature_args = std::make_unique<TemperatureTaskArgs>(measurement_queue, temperature.get());
    network_args = std::make_unique<NetworkTaskArgs>(measurement_queue, network.get());
    led_args = std::make_unique<LedTaskArgs>(led.get());

    // Register tasks to FreeRTOS
    // We pass the addresses in setup_args directly to xTaskCreate
    xTaskCreate(setup_task, "SETUP", 256, setup_args.get(), configMAX_PRIORITIES - 1, &setup_args->self);
    xTaskCreate(temperature_task, "TEMPERATURE", 256, temperature_args.get(), configMAX_PRIORITIES - 2,
        &setup_args->temperature_task);
    xTaskCreate(network_task, "NETWORK", 256, network_args.get(), configMAX_PRIORITIES - 3, &setup_args->network_task);
    xTaskCreate(led_task, "LED", configMINIMAL_STACK_SIZE, led_args.get(), configMAX_PRIORITIES - 4, nullptr);

    // Start the FreeRTOS scheduler
    utils::logger.info("Staring RTOS scheduler...\n");
    vTaskStartScheduler();
    utils::logger.error("RTOS scheduler returned! You don't have enough memory!\n");

    while (true) {
        ;
    }

    return -1; // This will never be reached
}