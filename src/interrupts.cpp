#include <atomic>
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
        flags.dma_complete = true;
    }
}

// USART2 Rx DMA1 channel 6
DMAISRFlags dma1_channel6_flags;
void dma1_channel6_isr(void) { dma_channel_isr(DMA1, DMA_CHANNEL6, dma1_channel6_flags); }

// USART2 Tx DMA1 channel 7
DMAISRFlags dma1_channel7_flags;
void dma1_channel7_isr(void) { dma_channel_isr(DMA1, DMA_CHANNEL7, dma1_channel7_flags); }

volatile std::atomic_bool usart2_overrun_error = false;
void usart2_isr(void)
{
    if (((USART_CR3(USART2) & USART_CR3_EIE) != 0) && ((USART_SR(USART2) & USART_SR_ORE) != 0)) {
        (void)USART2_DR;
        usart_disable_error_interrupt(USART2);
        usart2_overrun_error = true;
    }
}