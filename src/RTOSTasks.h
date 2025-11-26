#pragma once

#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include "BlinkyLED.h"
#include "Network.h"
#include "Temperature.h"

constexpr unsigned int measurement_queue_size = 10;

struct LedTaskArgs {
    const BlinkyLED* led;
    
    explicit LedTaskArgs(const BlinkyLED* led_ptr) : led(led_ptr) {}
};
void led_task(void* a);

struct TemperatureTaskArgs {
    QueueHandle_t measurement_queue;
    const TemperatureSensor* temperature;
};
void temperature_task(void* a);

struct NetworkTaskArgs {
    QueueHandle_t measurement_queue;
    Network* network;
};
void network_task(void* a);

struct SetupTaskArgs {
    xTaskHandle self;
    xTaskHandle temperature_task;
    xTaskHandle network_task;
    const TemperatureSensor* temperature;
    Network* network;
};
void setup_task(void* a);