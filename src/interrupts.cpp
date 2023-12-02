#include <cstdio>

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/dma.h>
#include <libopencm3/stm32/usart.h>

// USART2 Rx DMA1 channel 6.
void dma1_channel6_isr(void)
{
    if (dma_get_interrupt_flag(DMA1, DMA_CHANNEL6, DMA_TEIF)) {
        dma_clear_interrupt_flags(DMA1, DMA_CHANNEL6, DMA_TEIF);
        printf("usart2 dma rx error isr called!\n");
    }
    if (dma_get_interrupt_flag(DMA1, DMA_CHANNEL6, DMA_HTIF)) {
        dma_clear_interrupt_flags(DMA1, DMA_CHANNEL6, DMA_HTIF);
        printf("usart2 dma rx half isr called!\n");
    }
    if (dma_get_interrupt_flag(DMA1, DMA_CHANNEL6, DMA_TCIF)) {
        dma_clear_interrupt_flags(DMA1, DMA_CHANNEL6, DMA_TCIF);
        printf("usart2 dma rx complete isr called!\n");
    }
}