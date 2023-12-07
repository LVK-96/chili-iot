#pragma once

#include <memory>
#include <optional>

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>

#include "Peripheral.h"

enum class BluePillGPIOPort : uint32_t { A = GPIOA, B = GPIOB, C = GPIOC };

enum class GPIOMode : uint8_t {
    INPUT = GPIO_MODE_INPUT,
    OUTPUT_10_MHZ = GPIO_MODE_OUTPUT_10_MHZ,
    OUTPUT_2_MHZ = GPIO_MODE_OUTPUT_2_MHZ,
    OUTPUT_50_MHZ = GPIO_MODE_OUTPUT_50_MHZ,
};

enum class GPIOFunction : uint8_t {
    INPUT_ANALOG = GPIO_CNF_INPUT_ANALOG,
    INPUT_FLOAT = GPIO_CNF_INPUT_FLOAT,
    INPUT_PULL_UPDOWN = GPIO_CNF_INPUT_PULL_UPDOWN,
    OUTPUT_PUSHPULL = GPIO_CNF_OUTPUT_PUSHPULL,
    OUTPUT_OPENDRAIN = GPIO_CNF_OUTPUT_OPENDRAIN,
    OUTPUT_ALTFN_PUSHPULL = GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,
    OUTPUT_ALTFN_OPENDRAIN = GPIO_CNF_OUTPUT_ALTFN_OPENDRAIN,
};

class GPIOPort final : public Peripheral {
public:
    constexpr GPIOPort(BluePillGPIOPort port, rcc_periph_clken clken, rcc_periph_rst rst) noexcept
        : Peripheral(clken, rst)
        , port(static_cast<uint32_t>(port))
    {
    }

    // You cannot enable/disable GPIO ports
    void enable() const override { }
    void disable() const override { }

    // Pin access
    void set_pins(uint16_t pins) const;
    void clear_pins(uint16_t pins) const;
    void toggle_pins(uint16_t pins) const;
    [[nodiscard]] uint16_t get_pins(uint16_t pins) const;
    void setup_pins(uint16_t pins, GPIOMode mode, GPIOFunction function) const;

private:
    uint32_t port;
};

struct GPIOPin {
    constexpr GPIOPin(uint16_t pin_nro, const GPIOPort* port)
        : pin_nro(pin_nro)
        , port(port)
    {
    }
    uint16_t pin_nro;
    const GPIOPort* port;
};