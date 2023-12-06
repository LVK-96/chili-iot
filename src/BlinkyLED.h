#pragma once

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>

#include "GPIO.h"

class BlinkyLED {
public:
    constexpr BlinkyLED() noexcept = default;
    virtual void toggle() const = 0;
    virtual void on() const = 0;
    virtual void off() const = 0;
};

class GPIOLED final : public BlinkyLED {
public:
    constexpr GPIOLED(const GPIOPin* pin) noexcept
        : pin(pin)
    {
    }

    void toggle() const override;
    void on() const override;
    void off() const override;

private:
    const GPIOPin* pin;
};