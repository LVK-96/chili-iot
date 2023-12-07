#pragma once

#include <memory>

#include "BlinkyLED.h"
#include "Logger.h"
#include "Network.h"
#include "System.h"
#include "Temperature.h"

class SensorNode {
public:
    SensorNode(const BlinkyLED* led, const TemperatureSensor* temperature, const Network* network);
    void setup();
    [[noreturn]] void main_loop();

private:
    const BlinkyLED* led;
    const TemperatureSensor* temperature;
    const Network* network;

    void setup_temperature();
    void setup_network();
};