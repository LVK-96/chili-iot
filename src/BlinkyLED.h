#pragma once

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>

#include "GPIO.h"

class BlinkyLED {
public:
    BlinkyLED() {};
    virtual void toggle() const = 0;
    virtual void on() const = 0;
    virtual void off() const = 0;
};

class GPIOLED : public BlinkyLED {
public:
    GPIOLED(const GPIOPin& pin);
    void toggle() const override;
    void on() const override;
    void off() const override;

private:
    const GPIOPin& pin;
};