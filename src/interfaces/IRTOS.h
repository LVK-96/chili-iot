#pragma once

#include <cstdint>

class IRTOS {
public:
    virtual ~IRTOS() = default;

    virtual void delay(uint32_t ms) const = 0;
    [[nodiscard]] virtual uint32_t get_tick_count() const = 0;

    // Returns true if notification received, false if timeout
    [[nodiscard]] virtual bool task_notify_wait(uint32_t timeout_ms) const = 0;

    using TaskHandle = void*;
    [[nodiscard]] virtual TaskHandle get_current_task_handle() const = 0;
};
