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
    while (true) {
        // Blink the led
        led.toggle();

        // Read & send temperature
        if (auto read_temperature = temperature.read()) {
            if (network.publish_measurement(read_temperature.value())
                == sensor_node_system::ErrorCode::NETWORK_RESPONSE_NOT_OK_ERROR) {
                sensor_node_system::network_setup(); // Reset the network connection if we got a not OK response
            }
        } else {
            sensor_node_system::temperature_setup(); // Reset the temperature sensor if the reading failed
        }
    }
}
