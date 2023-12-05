#pragma once

#include <memory>

#include "BlinkyLED.h"
#include "Logger.h"
#include "Network.h"
#include "System.h"
#include "Temperature.h"

class SensorNode {
public:
    SensorNode(
        const BlinkyLED& led, const Logger& logger, const TemperatureSensor& temperature, const Network& network);
    [[noreturn]] void main_loop();

private:
    const BlinkyLED& led;
    const Logger& logger;
    const TemperatureSensor& temperature;
    const Network& network;
};