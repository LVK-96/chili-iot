#include <cstdio>
#include <format>
#include <iostream>
#include <memory>
#include <utility>

#include "BlinkyLED.h"
#include "Logger.h"
#include "SensorNode.h"
#include "System.h"
#include "Temperature.h"

SensorNode::SensorNode(const BlinkyLED& led, const Logger& logger, const TemperatureSensor& temperature)
    : led(led)
    , logger(logger)
    , temperature(temperature)
{
}

void SensorNode::main_loop()
{
    while (true) {
        // Blink the led
        led.toggle();

        // Read & log temperature
        auto read_temperature = temperature.read();
        if (read_temperature) {
            printf("Temperature: %.2lf\n", read_temperature.value());
        }

        // Wait a bit and do it again
        _system::sleep_ms(1000);
    }
}
