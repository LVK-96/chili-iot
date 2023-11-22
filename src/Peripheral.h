#pragma once

#include <libopencm3/stm32/rcc.h>

class Peripheral {
public:
    Peripheral(rcc_periph_clken clken, rcc_periph_rst rst)
        : clken(clken)
        , rst(rst)
    {
    }
    virtual ~Peripheral() = 0;
    virtual void enable() = 0;
    virtual void disable() = 0;

    void clk_enable() const
    {
        rcc_periph_clock_enable(clken);
    }

    void clk_disable() const
    {
        rcc_periph_clock_disable(clken);
    }

    void reset() const
    {
        rcc_periph_reset_hold(rst);
    }

    void reset_release() const
    {
        rcc_periph_reset_release(rst);
    }

    void reset_pulse() const
    {
        rcc_periph_reset_pulse(rst);
    }

private:
    const rcc_periph_clken clken;
    const rcc_periph_rst rst;
};

inline Peripheral::~Peripheral() { }