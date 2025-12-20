#pragma once

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>

#include "GPIO.h"
#include "interfaces/ILED.h"

class BlinkyLED final : public ILED {
public:
// GCC analyzer gives a analyzer-possible-null-dereference false positive here when called from std::make_unique
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wanalyzer-possible-null-dereference"
    constexpr BlinkyLED(const GPIOPin* pin) noexcept
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