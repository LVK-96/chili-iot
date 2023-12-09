#include <FreeRTOS.h>
#include <task.h>

#include "RTOSTasks.h"
#include "System.h"

int main()
{
    // Setup the Bluepill board
    bluepill::setup();
    utils::logger.info("Board setup OK!\n");

    // Regiter tasks to FreeRTOS
    xTaskCreate(led_task, "LED", configMINIMAL_STACK_SIZE, nullptr, configMAX_PRIORITIES - 2, nullptr);
    xTaskCreate(temperature_task, "TEMPERATURE", 1024, nullptr, configMAX_PRIORITIES - 1, nullptr);

    // Start the FreeRTOS scheduler
    utils::logger.info("Staring RTOS scheduler...\n");
    vTaskStartScheduler();
    utils::logger.error("RTOS scheduler returned! Maybe it ran out of heap...\n");

    while (true) {
        ;
    }

    return -1; // This will never be reached
}