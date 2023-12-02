#include "DMA.h"

void DMA::setup_channel_from_peripheral_to_memory(BluePillDMAChannel channel, uint32_t peripheral_addr,
    BluePillDMAPeripheralWordSize peripheral_word_size, BluePillDMAMemWordSize memory_word_size,
    BluePillDMAPriority priority, std::span<uint8_t> dest_buf, bool increment_peripheral, bool increment_mem,
    bool transfer_error_interrupt, bool half_transfer_interrupt, bool transfer_complete_interrupt) const
{
    uint32_t channel_uint32 = static_cast<uint32_t>(channel);

    // Peripheral to memory mode
    dma_set_read_from_peripheral(dma, channel_uint32);
    dma_set_peripheral_size(dma, channel_uint32, static_cast<uint32_t>(peripheral_word_size));
    dma_set_memory_size(dma, channel_uint32, static_cast<uint32_t>(memory_word_size));
    dma_set_priority(dma, channel_uint32, static_cast<uint32_t>(priority));

    // Increment mode, first read/write buf[0], then buf[1]...
    // No increment mode, always read from the same place
    if (increment_peripheral) {
        dma_enable_peripheral_increment_mode(dma, channel_uint32);
    } else {
        dma_disable_peripheral_increment_mode(dma, channel_uint32);
    }
    if(increment_mem) {
        dma_enable_memory_increment_mode(dma, channel_uint32);
    } else {
        dma_disable_memory_increment_mode(dma, channel_uint32);
    }


    // BluePillDMAPeripheralWordSize underlying values are magic numbers that need to be written into CSRs
    // Get the actual word size in bytes
    unsigned int peripheral_word_size_bytes = 1; // BYTE
    if (peripheral_word_size == BluePillDMAPeripheralWordSize::HALFWORD) {
        peripheral_word_size_bytes = 2;
    } else if (peripheral_word_size == BluePillDMAPeripheralWordSize::WORD) {
        peripheral_word_size_bytes = 4;
    }
    // Setup transfer source, destination and size
    dma_set_peripheral_address(dma, channel_uint32, peripheral_addr);
    unsigned int number_of_words_to_transfer = dest_buf.size_bytes() / peripheral_word_size_bytes;
    dma_set_number_of_data(dma, channel_uint32, number_of_words_to_transfer);
    dma_set_memory_address(dma, channel_uint32, reinterpret_cast<uint32_t>(dest_buf.data()));

    // Interrupts
    if (transfer_error_interrupt){
        dma_enable_transfer_error_interrupt(dma, channel_uint32);
    } else {
        dma_disable_transfer_error_interrupt(dma, channel_uint32);
    }
    if (half_transfer_interrupt) {
        dma_enable_half_transfer_interrupt(dma, channel_uint32);
    } else {
        dma_disable_half_transfer_interrupt(dma, channel_uint32);
    }
    if (transfer_complete_interrupt) {
        dma_enable_transfer_complete_interrupt(dma, channel_uint32);
    } else {
        dma_disable_transfer_complete_interrupt(dma, channel_uint32);
    }
}
void DMA::enable() const {}

void DMA::disable() const
{
    for (auto &channel : all_channels) {
        dma_disable_channel(dma, channel);
    }
}

void DMA::enable(BluePillDMAChannel channel) const
{
    dma_enable_channel(dma, static_cast<uint8_t>(channel));
}

void DMA::disable(BluePillDMAChannel channel) const
{
    dma_disable_channel(dma, static_cast<uint8_t>(channel));
}
