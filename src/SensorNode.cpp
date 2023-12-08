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
    unsigned int failures = 0;
    while (network->connect_to_ap() != sensor_node_system::ErrorCode::OK) {
        if (failures > 0) {
            network->hard_reset();
        } else {
            network->reset();
        }
        failures++;
    }

    network->connect_to_server();
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
            network->publish_measurement(read_temperature.value());
        } else {
            setup_temperature(); // Reset the temperature sensor if the reading failed
        }

        // TODO: add a periodic test that we are still connected to the server

        sensor_node_system::sleep_ms(1000);
    }
}
