#pragma once

#include <memory>

#include "BlinkyLED.h"
#include "Logger.h"
#include "System.h"
#include "Temperature.h"

class SensorNode {
private:
    const BlinkyLED& led;
    const Logger& logger;
    const TemperatureSensor& temperature;

public:
    SensorNode(const BlinkyLED& led, const Logger& logger,
        const TemperatureSensor& temperature);
    [[noreturn]] void main_loop();
};