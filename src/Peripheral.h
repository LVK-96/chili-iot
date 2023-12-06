#pragma once

#include <libopencm3/stm32/rcc.h>

class BasePeripheral {
public:
    virtual void enable() const = 0;
    virtual void disable() const = 0;
};

class NoResetPeripheral : public BasePeripheral {
public:
    constexpr NoResetPeripheral(rcc_periph_clken clken) noexcept
        : clken(clken)
    {
    }
    void clk_enable() const { rcc_periph_clock_enable(clken); }
    void clk_disable() const { rcc_periph_clock_disable(clken); }

private:
    rcc_periph_clken clken;
};

class Peripheral : public NoResetPeripheral {
public:
    constexpr Peripheral(rcc_periph_clken clken, rcc_periph_rst rst) noexcept
        : NoResetPeripheral(clken)
        , rst(rst)
    {
    }
    void reset() const { rcc_periph_reset_hold(rst); }
    void reset_release() const { rcc_periph_reset_release(rst); }
    void reset_pulse() const { rcc_periph_reset_pulse(rst); }

private:
    rcc_periph_rst rst;
};