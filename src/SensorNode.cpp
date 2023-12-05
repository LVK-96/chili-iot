#include <cstdio>
#include <memory>
#include <utility>

#include "BlinkyLED.h"
#include "Logger.h"
#include "Network.h"
#include "SensorNode.h"
#include "System.h"
#include "Temperature.h"

SensorNode::SensorNode(
    const BlinkyLED& led, const Logger& logger, const TemperatureSensor& temperature, const Network& network)
    : led(led)
    , logger(logger)
    , temperature(temperature)
    , network(network)
{
}

void SensorNode::main_loop()
{
    network.reset(); // Reset the network device
    network.connect_to_ap(); // Connect to the network
    while (true) {
        // Blink the led
        led.toggle();

        // Read & log temperature
        auto read_temperature = temperature.read();
        if (read_temperature) {
            printf("Temperature: %.2lf\n", read_temperature.value());
        }

        // Wait a bit and do it again
        sensor_node_system::sleep_ms(1000);
    }
}
