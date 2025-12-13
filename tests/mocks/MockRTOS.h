#pragma once

#include "interfaces/IRTOS.h"
#include <vector>

class MockRTOS : public IRTOS {
public:
    mutable std::vector<uint32_t> delays;
    mutable std::vector<uint32_t> notify_waits;
    mutable bool next_notify_wait_result = true;

    void delay(uint32_t ms) const override { delays.push_back(ms); }

    uint32_t get_tick_count() const override { return 0; }

    bool task_notify_wait(uint32_t timeout_ms) const override
    {
        notify_waits.push_back(timeout_ms);
        return next_notify_wait_result;
    }

    TaskHandle get_current_task_handle() const override { return reinterpret_cast<TaskHandle>(0xDEADBEEF); }
};
