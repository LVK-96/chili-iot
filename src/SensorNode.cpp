#include <cstdio>
#include <memory>
#include <utility>

#include "BlinkyLED.h"
#include "Logger.h"
#include "SensorNode.h"
#include "System.h"
#include "Temperature.h"
#include "Network.h"

SensorNode::SensorNode(const BlinkyLED& led, const Logger& logger, const TemperatureSensor& temperature, const Network &network)
    : led(led)
    , logger(logger)
    , temperature(temperature)
    , network(network)
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
        printf("AT OK: %x\n", network.test_connection());

        // Wait a bit and do it again
        sensor_node_system::sleep_ms(1000);
    }
}
