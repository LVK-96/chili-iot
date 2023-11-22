#include <cstdint>
#include <optional>

#include "GPIO.h"

GPIOPort::GPIOPort(BluePillGPIOPort port, rcc_periph_clken clken, rcc_periph_rst rst)
    : Peripheral(clken, rst)
    , port(static_cast<uint32_t>(port))
{
}

void GPIOPort::set_pins(uint16_t pins) const
{
    gpio_set(port, pins);
}

void GPIOPort::clear_pins(uint16_t pins) const
{
    gpio_clear(port, pins);
}

void GPIOPort::toggle_pins(uint16_t pins) const
{
    gpio_toggle(port, pins);
}

uint16_t GPIOPort::get_pins(uint16_t pins) const
{
    return gpio_get(port, pins);
}

void GPIOPort::setup_pins(uint16_t pins, GPIOMode mode, GPIOFunction function)
{
    gpio_set_mode(port, static_cast<uint8_t>(mode), static_cast<uint8_t>(function), pins);
}