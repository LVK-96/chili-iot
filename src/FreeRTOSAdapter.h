#pragma once

#include "interfaces/IRTOS.h"

class FreeRTOSAdapter final : public IRTOS {
public:
    void delay(uint32_t ms) const override;
    [[nodiscard]] uint32_t get_tick_count() const override;
    [[nodiscard]] bool task_notify_wait(uint32_t timeout_ms) const override;
    [[nodiscard]] TaskHandle get_current_task_handle() const override;
};
