#include <cstdio>
#include <memory>
#include <utility>

#include <FreeRTOS.h>
#include <task.h>

#include "RTOSTasks.h"
#include "utils.h"

static void setup_network(const Network& network)
{
    network.reset();
    while (network.connect_to_ap() != utils::ErrorCode::OK) {
        network.hard_reset();
    }

    network.connect_to_server();
}

static void setup_temperature(const TemperatureSensor& temperature)
{
    while (temperature.init() != utils::ErrorCode::OK) {
        ;
    }
}

static void setup(const TemperatureSensor& temperature, const Network& network)
{
    setup_temperature(temperature);
    setup_network(network);
}

void led_task([[maybe_unused]] void* a)
{
    GPIOLED led { &bluepill::peripherals::led_pin };
    while (true) {
        // Blink the led
        led.toggle();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void temperature_task([[maybe_unused]] void* a)
{
    BME280TemperatureSensor temperature { &bluepill::peripherals::i2c1,
        BME280I2CBusAddr::SECONDARY }; // The Waveshare BME280 module defaults to the secondary I2C address (0x77)
    ESP8266Network network { &bluepill::peripherals::usart2, &bluepill::peripherals::esp_reset_pin };

    setup(temperature, network);

    while (true) {
        // Read & send temperature
        if (auto read_temperature = temperature.read()) {
            network.publish_measurement(read_temperature.value());
        } else {
            setup_temperature(temperature);
        }
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}

void vApplicationStackOverflowHook([[maybe_unused]] TaskHandle_t xTask, [[maybe_unused]] char* pcTaskName)
{
    while (true) {
        utils::logger.log("STACK OVERFLOW!\n");
        utils::nop(10000000);
    }
}