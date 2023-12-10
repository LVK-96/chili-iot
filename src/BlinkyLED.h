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
// GCC analyzer gives a analyzer-possible-null-dereference false positive here when called from std::make_unique
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wanalyzer-possible-null-dereference"
    constexpr GPIOLED(const GPIOPin* pin) noexcept
        : pin(pin)
    {
    }
#pragma GCC diagnostic pop

    void toggle() const override;
    void on() const override;
    void off() const override;

private:
    const GPIOPin* pin;
};