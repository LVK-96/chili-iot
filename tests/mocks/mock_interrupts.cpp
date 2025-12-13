#include "interrupts.h"

DMAISRFlags dma1_channel6_flags;
DMAISRFlags dma1_channel7_flags;

volatile std::atomic_bool usart2_overrun_error = false;
volatile std::atomic_bool usart2_tx_transfer_complete = false;

volatile std::atomic_uint32_t systick_counter = 0;
