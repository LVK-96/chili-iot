#include <cstdio>
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
            char buf[20];
            snprintf(buf, 20, "Temperature: %.2lf\n", read_temperature.value());
            logger.log(buf);
        }

        // Wait a bit and do it again
        _system::sleep_ms(1000);
    }
}
