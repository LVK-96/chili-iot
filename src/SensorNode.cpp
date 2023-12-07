#include <cstdio>
#include <memory>
#include <utility>

#include "BlinkyLED.h"
#include "Logger.h"
#include "Network.h"
#include "SensorNode.h"
#include "System.h"
#include "Temperature.h"

SensorNode::SensorNode(const BlinkyLED* led, const TemperatureSensor* temperature, const Network* network)
    : led(led)
    , temperature(temperature)
    , network(network)
{
}

void SensorNode::setup_network()
{
    do {
        network->reset();
    } while (network->connect_to_ap() != sensor_node_system::ErrorCode::OK);
}

void SensorNode::setup_temperature()
{
    while (temperature->init() != sensor_node_system::ErrorCode::OK) {
        ;
    }
}

void SensorNode::setup()
{
    setup_temperature();
    setup_network();
}

void SensorNode::main_loop()
{
    setup();
    while (true) {
        // Blink the led
        led->toggle();

        // Read & send temperature
        if (auto read_temperature = temperature->read()) {
            if (network->publish_measurement(read_temperature.value())
                == sensor_node_system::ErrorCode::NETWORK_RESPONSE_NOT_OK_ERROR) {
                setup_network(); // Reset the network connection if we got a not OK response
            }
        } else {
            setup_temperature(); // Reset the temperature sensor if the reading failed
        }
    }
}
