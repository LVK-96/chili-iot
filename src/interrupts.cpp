#include <atomic>

#include <FreeRTOS.h>
#include <task.h>

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/dma.h>
#include <libopencm3/stm32/usart.h>

#include "interrupts.h"

static bool get_dma_interrupt_flag(uint32_t dma, uint8_t channel, uint32_t flag)
{
    return ((DMA_CCR(dma, channel) & flag) != 0);
}

static bool get_dma_complete_interrupt_enable_flag(uint32_t dma, uint8_t channel)
{
    return get_dma_interrupt_flag(dma, channel, DMA_CCR_TCIE);
}

static bool get_dma_half_interrupt_enable_flag(uint32_t dma, uint8_t channel)
{
    return get_dma_interrupt_flag(dma, channel, DMA_CCR_HTIE);
}

static bool get_dma_error_interrupt_enable_flag(uint32_t dma, uint8_t channel)
{
    return get_dma_interrupt_flag(dma, channel, DMA_CCR_TEIE);
}

static TaskHandle_t network_task = nullptr;

static void dma_channel_isr(uint32_t dma, uint8_t channel, DMAISRFlags& flags)
{
    bool const transfer_error_interrupt
        = dma_get_interrupt_flag(dma, channel, DMA_TEIF) && get_dma_error_interrupt_enable_flag(dma, channel);
    bool const half_interrupt
        = dma_get_interrupt_flag(dma, channel, DMA_HTIF) && get_dma_half_interrupt_enable_flag(dma, channel);
    bool const transfer_complete_interrupt
        = dma_get_interrupt_flag(dma, channel, DMA_TCIF) && get_dma_complete_interrupt_enable_flag(dma, channel);

    if (transfer_error_interrupt) {
        dma_clear_interrupt_flags(dma, channel, DMA_TEIF);
        flags.dma_error = true;
    }

    if (half_interrupt) {
        dma_clear_interrupt_flags(dma, channel, DMA_HTIF);
        flags.dma_half = true;
    }

    if (transfer_complete_interrupt) {
        dma_clear_interrupt_flags(dma, channel, DMA_TCIF);
        dma_disable_channel(dma, channel);

        // Notify the network task that a TX DMA transfer is complete
        if (network_task != nullptr && channel == DMA_CHANNEL7) {
            BaseType_t higher_prio_task_woken = pdFALSE;
            xTaskNotifyFromISR(network_task, 0, eNoAction, &higher_prio_task_woken);
            portYIELD_FROM_ISR(higher_prio_task_woken);
        }

        flags.dma_complete = true;
    }
}

void set_network_task_handle_for_interrupts(TaskHandle_t task) { network_task = task; }

// USART2 Rx DMA1 channel 6
DMAISRFlags dma1_channel6_flags;
void dma1_channel6_isr(void) { dma_channel_isr(DMA1, DMA_CHANNEL6, dma1_channel6_flags); }

// USART2 Tx DMA1 channel 7
DMAISRFlags dma1_channel7_flags;
void dma1_channel7_isr(void) { dma_channel_isr(DMA1, DMA_CHANNEL7, dma1_channel7_flags); }

volatile std::atomic_bool usart2_overrun_error = false;
volatile std::atomic_bool usart2_idle_line_received = false;
void usart2_isr(void)
{
    bool overrun_error_interrupt
        = ((USART_CR3(USART2) & USART_CR3_EIE) != 0) && ((USART_SR(USART2) & USART_SR_ORE) != 0);
    bool idle_line_received_interrupt
        = ((USART_CR1(USART2) & USART_CR1_IDLEIE) != 0) && ((USART_SR(USART2) & USART_SR_IDLE) != 0);

    if (overrun_error_interrupt) {
        USART_SR(USART2) &= ~USART_SR_ORE;
        usart2_overrun_error = true;
    }

    if (idle_line_received_interrupt) {
        USART_SR(USART2) &= ~USART_SR_IDLE;
        usart2_idle_line_received = true;
    }
}