#include "interrupts.h"

DMAISRFlags dma1_channel6_flags;
DMAISRFlags dma1_channel7_flags;

volatile std::atomic_bool usart2_overrun_error = false;
volatile std::atomic_bool usart2_tx_transfer_complete = false;

volatile std::atomic_uint32_t systick_counter = 0;

void set_network_task_handle_for_tx_dma_interrupts(TaskHandle_t task) { }
void set_network_task_handle_for_rx_dma_interrupts(TaskHandle_t task) { }
void set_network_task_handle_for_usart2_interrupts(TaskHandle_t task) { }
