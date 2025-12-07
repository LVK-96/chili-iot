#include "DMA.h"

// No "dma_disable_circular_mode" function in libopencm3, let's just clear the bit manually
static void dma_disable_circular_mode(uint32_t dma, uint8_t channel) { DMA_CCR(dma, channel) &= ~DMA_CCR_CIRC; }

void DMA::setup_channel(DMADirection direction, BluePillDMAChannel channel, uint32_t peripheral_addr,
    BluePillDMAPeripheralWordSize peripheral_word_size, uint32_t memory_addr, BluePillDMAMemWordSize memory_word_size,
    BluePillDMAPriority priority, unsigned int number_of_data, bool increment_peripheral, bool increment_mem,
    bool transfer_error_interrupt, bool half_transfer_interrupt, bool transfer_complete_interrupt,
    bool circular_mode) const
{
    auto const channel_uint8 = static_cast<uint8_t>(channel);

    if (direction == DMADirection::PER2MEM) {
        dma_set_read_from_peripheral(dma, channel_uint8);
    } else if (direction == DMADirection::MEM2PER) {
        dma_set_read_from_memory(dma, channel_uint8);
    } else { // MEM2MEM
        dma_set_read_from_memory(dma, channel_uint8);
        dma_enable_mem2mem_mode(dma, channel_uint8);
    }

    dma_set_peripheral_size(dma, channel_uint8, static_cast<uint32_t>(peripheral_word_size));
    dma_set_memory_size(dma, channel_uint8, static_cast<uint32_t>(memory_word_size));
    dma_set_priority(dma, channel_uint8, static_cast<uint32_t>(priority));

    // Increment mode, first read/write buf[0], then buf[1]...
    // No increment mode, always read from the same place
    if (increment_peripheral) {
        dma_enable_peripheral_increment_mode(dma, channel_uint8);
    } else {
        dma_disable_peripheral_increment_mode(dma, channel_uint8);
    }
    if (increment_mem) {
        dma_enable_memory_increment_mode(dma, channel_uint8);
    } else {
        dma_disable_memory_increment_mode(dma, channel_uint8);
    }

    // Setup transfer source, destination and size
    dma_set_peripheral_address(dma, channel_uint8, peripheral_addr);
    dma_set_number_of_data(dma, channel_uint8, number_of_data);
    dma_set_memory_address(dma, channel_uint8, memory_addr);

    // Interrupts
    if (transfer_error_interrupt) {
        dma_enable_transfer_error_interrupt(dma, channel_uint8);
    } else {
        dma_disable_transfer_error_interrupt(dma, channel_uint8);
    }
    if (half_transfer_interrupt) {
        dma_enable_half_transfer_interrupt(dma, channel_uint8);
    } else {
        dma_disable_half_transfer_interrupt(dma, channel_uint8);
    }
    if (transfer_complete_interrupt) {
        dma_enable_transfer_complete_interrupt(dma, channel_uint8);
    } else {
        dma_disable_transfer_complete_interrupt(dma, channel_uint8);
    }

    if (circular_mode) {
        dma_enable_circular_mode(dma, channel_uint8);
    } else {
        dma_disable_circular_mode(dma, channel_uint8);
    }
}

void DMA::enable() const { }

void DMA::disable() const
{
    for (const auto& channel : all_channels) {
        dma_disable_channel(dma, channel);
    }
}

void DMA::enable(BluePillDMAChannel channel) const { dma_enable_channel(dma, static_cast<uint8_t>(channel)); }

void DMA::disable(BluePillDMAChannel channel) const { dma_disable_channel(dma, static_cast<uint8_t>(channel)); }

void DMA::reset() const
{
    for (const auto& channel : all_channels) {
        dma_channel_reset(dma, channel);
    }
}

void DMA::reset(BluePillDMAChannel channel) const { dma_channel_reset(dma, static_cast<uint8_t>(channel)); }

unsigned int DMA::get_count(BluePillDMAChannel channel) const { return DMA_CNDTR(dma, static_cast<uint8_t>(channel)); }