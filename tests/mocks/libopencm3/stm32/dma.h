#pragma once

#include <cstdint>

#define DMA1 0x40020000
#define DMA2 0x40020400

#define DMA_CHANNEL1 1
#define DMA_CHANNEL2 2
#define DMA_CHANNEL3 3
#define DMA_CHANNEL4 4
#define DMA_CHANNEL5 5
#define DMA_CHANNEL6 6
#define DMA_CHANNEL7 7

#define DMA_CCR_MINC (1 << 7)
#define DMA_CCR_PINC (1 << 6)
#define DMA_CCR_CIRC (1 << 5)
#define DMA_CCR_DIR (1 << 4)
#define DMA_CCR_TCIE (1 << 1)
#define DMA_CCR_EN (1 << 0)
#define DMA_CCR_MEM2MEM (1 << 14)

#define DMA_CCR_MSIZE_8BIT (0x0 << 10)
#define DMA_CCR_MSIZE_16BIT (0x1 << 10)
#define DMA_CCR_MSIZE_32BIT (0x2 << 10)

#define DMA_CCR_PSIZE_8BIT (0x0 << 8)
#define DMA_CCR_PSIZE_16BIT (0x1 << 8)
#define DMA_CCR_PSIZE_32BIT (0x2 << 8)

#define DMA_CCR_PL_LOW (0x0 << 12)
#define DMA_CCR_PL_MEDIUM (0x1 << 12)
#define DMA_CCR_PL_HIGH (0x2 << 12)
#define DMA_CCR_PL_VERY_HIGH (0x3 << 12)

void dma_channel_reset(uint32_t dma, uint8_t channel);
void dma_set_peripheral_address(uint32_t dma, uint8_t channel, uint32_t address);
void dma_set_memory_address(uint32_t dma, uint8_t channel, uint32_t address);
void dma_set_number_of_data(uint32_t dma, uint8_t channel, uint16_t number);
void dma_set_read_from_peripheral(uint32_t dma, uint8_t channel);
void dma_set_read_from_memory(uint32_t dma, uint8_t channel);
void dma_enable_memory_increment_mode(uint32_t dma, uint8_t channel);
void dma_disable_memory_increment_mode(uint32_t dma, uint8_t channel);
void dma_enable_peripheral_increment_mode(uint32_t dma, uint8_t channel);
void dma_disable_peripheral_increment_mode(uint32_t dma, uint8_t channel);
void dma_enable_circular_mode(uint32_t dma, uint8_t channel);
void dma_enable_mem2mem_mode(uint32_t dma, uint8_t channel);
void dma_set_peripheral_size(uint32_t dma, uint8_t channel, uint32_t size);
void dma_set_memory_size(uint32_t dma, uint8_t channel, uint32_t size);
void dma_set_priority(uint32_t dma, uint8_t channel, uint32_t priority);
void dma_enable_transfer_complete_interrupt(uint32_t dma, uint8_t channel);
void dma_disable_transfer_complete_interrupt(uint32_t dma, uint8_t channel);
void dma_enable_transfer_error_interrupt(uint32_t dma, uint8_t channel);
void dma_disable_transfer_error_interrupt(uint32_t dma, uint8_t channel);
void dma_enable_half_transfer_interrupt(uint32_t dma, uint8_t channel);
void dma_disable_half_transfer_interrupt(uint32_t dma, uint8_t channel);
void dma_enable_channel(uint32_t dma, uint8_t channel);
void dma_disable_channel(uint32_t dma, uint8_t channel);

extern uint32_t mock_dma_cndtr;
extern uint32_t mock_dma_ccr;
#define DMA_CNDTR(dma, channel) ((void)(dma), (void)(channel), mock_dma_cndtr)
#define DMA_CCR(dma, channel) ((void)(dma), (void)(channel), mock_dma_ccr)

#define DMA_CCR_HTIE (1 << 2)
#define DMA_CCR_TEIE (1 << 3)
#define DMA_CCR_TCIE (1 << 1)

#define DMA_TEIF (1 << 3)
#define DMA_HTIF (1 << 2)
#define DMA_TCIF (1 << 1)

bool dma_get_interrupt_flag(uint32_t dma, uint8_t channel, uint32_t flag);
void dma_clear_interrupt_flags(uint32_t dma, uint8_t channel, uint32_t flags);
