#pragma once

#include <array>
#include <atomic>
#include <cstdint>

extern volatile std::atomic_bool dma_buffer_full;
extern volatile std::atomic_bool dma_buffer_half;
extern volatile std::atomic_bool dma_error;

extern volatile std::atomic_bool usart2_overrun_error;

extern volatile std::atomic_uint32_t systick_counter;