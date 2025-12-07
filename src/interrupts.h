#pragma once

#include <atomic>

#include <FreeRTOS.h>
#include <task.h>

struct DMAISRFlags {
    volatile std::atomic_bool dma_complete = false;
    volatile std::atomic_bool dma_half = false;
    volatile std::atomic_bool dma_error = false;
};

extern DMAISRFlags dma1_channel6_flags;
extern DMAISRFlags dma1_channel7_flags;

extern volatile std::atomic_bool usart2_overrun_error;

extern volatile std::atomic_uint32_t systick_counter;

void set_network_task_handle_for_interrupts(TaskHandle_t task);