#include <atomic>
#include <cstdio>
#include <functional>

#include <FreeRTOS.h>
#include <task.h>

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/dma.h>
#include <libopencm3/stm32/usart.h>

#include "Logger.h"
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

// Not quite sure why these would ever be different tasks, but just in case :D
namespace {
TaskHandle_t network_tx_dma_task = nullptr;
TaskHandle_t network_rx_dma_task = nullptr;
TaskHandle_t network_usart2_task = nullptr;
}
void set_network_task_handle_for_tx_dma_interrupts(TaskHandle_t task) { network_tx_dma_task = task; }
void set_network_task_handle_for_rx_dma_interrupts(TaskHandle_t task) { network_rx_dma_task = task; }
void set_network_task_handle_for_usart2_interrupts(TaskHandle_t task) { network_usart2_task = task; }

static void dma_channel_isr(uint32_t dma, uint8_t channel, DMAISRFlags& flags)
{
    bool const transfer_error_interrupt
        = dma_get_interrupt_flag(dma, channel, DMA_TEIF) && get_dma_error_interrupt_enable_flag(dma, channel);
    bool const half_interrupt
        = dma_get_interrupt_flag(dma, channel, DMA_HTIF) && get_dma_half_interrupt_enable_flag(dma, channel);
    bool const transfer_complete_interrupt
        = dma_get_interrupt_flag(dma, channel, DMA_TCIF) && get_dma_complete_interrupt_enable_flag(dma, channel);

    BaseType_t higher_prio_task_woken = pdFALSE;

    std::function notify_network_task = [channel, &higher_prio_task_woken](TaskHandle_t task) {
        if (task != nullptr && channel == DMA_CHANNEL7) {
            xTaskNotifyFromISR(task, 0, eNoAction, &higher_prio_task_woken);
        }
    };

    if (transfer_error_interrupt) {
        dma_clear_interrupt_flags(dma, channel, DMA_TEIF);
        flags.dma_error = true;
        notify_network_task(network_rx_dma_task);
        notify_network_task(network_tx_dma_task);
    }

    if (half_interrupt) {
        dma_clear_interrupt_flags(dma, channel, DMA_HTIF);
        flags.dma_half = true;
    }

    if (transfer_complete_interrupt) {
        dma_clear_interrupt_flags(dma, channel, DMA_TCIF);
        flags.dma_complete = true;
    }

    portYIELD_FROM_ISR(higher_prio_task_woken);
}

// USART2 Rx DMA1 channel 6
DMAISRFlags dma1_channel6_flags;
void dma1_channel6_isr(void) { dma_channel_isr(DMA1, DMA_CHANNEL6, dma1_channel6_flags); }

// USART2 Tx DMA1 channel 7
DMAISRFlags dma1_channel7_flags;
void dma1_channel7_isr(void) { dma_channel_isr(DMA1, DMA_CHANNEL7, dma1_channel7_flags); }

volatile std::atomic_bool usart2_overrun_error = false;
volatile std::atomic_bool usart2_tx_transfer_complete = false;

void usart2_isr(void)
{
    bool overrun_error_interrupt
        = ((USART_CR3(USART2) & USART_CR3_EIE) != 0) && ((USART_SR(USART2) & USART_SR_ORE) != 0);
    bool transfer_complete_interrupt
        = ((USART_CR1(USART2) & USART_CR1_TCIE) != 0) && ((USART_SR(USART2) & USART_SR_TC) != 0);

    BaseType_t higher_prio_task_woken = pdFALSE;

    if (overrun_error_interrupt) {
        USART_SR(USART2) &= ~USART_SR_ORE;
        usart2_overrun_error = true;
        if (network_usart2_task != nullptr) {
            xTaskNotifyFromISR(network_usart2_task, 0, eNoAction, &higher_prio_task_woken);
        }
    }

    if (transfer_complete_interrupt) {
        // Clear the TC bit in DMA transmit mode
        if (USART_CR3(USART2) & USART_CR3_DMAT) {
            USART_SR(USART2) &= ~USART_SR_TC;
        }
        usart2_tx_transfer_complete = true;
        if (network_usart2_task != nullptr) {
            xTaskNotifyFromISR(network_usart2_task, 0, eNoAction, &higher_prio_task_woken);
        }
    }

    portYIELD_FROM_ISR(higher_prio_task_woken);
}

void hard_fault_handler(void)
{
    utils::logger.error("HARD FAULT!!!\n");
    while (true)
        ;
}