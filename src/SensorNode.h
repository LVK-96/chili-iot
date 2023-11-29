#pragma once

#include <memory>

#include "BlinkyLED.h"
#include "Logger.h"
#include "System.h"
#include "Temperature.h"
#include "Network.h"

class SensorNode {
private:
    const BlinkyLED& led;
    const Logger& logger;
    const TemperatureSensor& temperature;
    const Network& network;

public:
    SensorNode(const BlinkyLED& led, const Logger& logger, const TemperatureSensor& temperature, const Network &network);
    [[noreturn]] void main_loop();
};