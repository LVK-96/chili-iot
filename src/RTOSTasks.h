#pragma once

#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include "BlinkyLED.h"
#include "interfaces/INetwork.h"
#include "interfaces/ITemperatureSensor.h"

constexpr unsigned int measurement_queue_size = 10;

struct LedTaskArgs {
    const ILED* led;

    explicit LedTaskArgs(const ILED* led_ptr)
        : led(led_ptr)
    {
    }
};
void led_task(void* a);

struct TemperatureTaskArgs {
    QueueHandle_t measurement_queue;
    const ITemperatureSensor* temperature;
};
void temperature_task(void* a);

struct NetworkTaskArgs {
    QueueHandle_t measurement_queue;
    INetwork* network;
};
void network_task(void* a);

struct SetupTaskArgs {
    xTaskHandle self;
    xTaskHandle temperature_task;
    xTaskHandle network_task;
    const ITemperatureSensor* temperature;
    INetwork* network;
};
void setup_task(void* a);