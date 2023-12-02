#include <span>
#include <cstdint>

#include <libopencm3/stm32/dma.h>

#include "Peripheral.h"

enum class BluePillDMAController : uint32_t
{
    _1 = DMA1,
    _2 = DMA2
};

enum class BluePillDMAChannel : uint8_t
{
    _1 = DMA_CHANNEL1,
    _2 = DMA_CHANNEL2,
    _3 = DMA_CHANNEL3,
    _4 = DMA_CHANNEL4,
    _5 = DMA_CHANNEL5,
    _6 = DMA_CHANNEL6,
    _7 = DMA_CHANNEL7
};

enum class BluePillDMAPeripheralWordSize
{
    BYTE = DMA_CCR_PSIZE_8BIT,
    HALFWORD = DMA_CCR_PSIZE_16BIT,
    WORD = DMA_CCR_PSIZE_32BIT
};

enum class BluePillDMAMemWordSize
{
    BYTE = DMA_CCR_MSIZE_8BIT,
    HALFWORD = DMA_CCR_MSIZE_16BIT,
    WORD = DMA_CCR_MSIZE_32BIT
};

enum class BluePillDMAPriority
{
    LOW = DMA_CCR_PL_LOW,
    MEDIUM = DMA_CCR_PL_MEDIUM,
    HIGH = DMA_CCR_PL_HIGH,
    VERY_HIGH = DMA_CCR_PL_VERY_HIGH
};

class DMA : public NoResetPeripheral
{
    public:
        constexpr DMA(BluePillDMAController dma, rcc_periph_clken clken) noexcept
            : NoResetPeripheral(clken), dma(static_cast<uint32_t>(dma)) {}
        void setup_channel_from_peripheral_to_memory(BluePillDMAChannel channel, uint32_t peripheral_addr,
            BluePillDMAPeripheralWordSize peripheral_word_size, BluePillDMAMemWordSize memory_word_size,
            BluePillDMAPriority priority, std::span<uint8_t> dest_buf, bool increment_peripheral, bool increment_mem,
            bool transfer_error_interrupt, bool half_transfer_interrupt, bool transfer_complete_interrupt) const;

        void enable() const override;
        void disable() const override;
        void enable(BluePillDMAChannel channel) const;
        void disable(BluePillDMAChannel channel) const;
        void reset() const {}; // DMA peripheral cannot be reset
    private:
        static constexpr std::array<const uint8_t, 7> all_channels = {
            static_cast<uint8_t>(BluePillDMAChannel::_1),
            static_cast<uint8_t>(BluePillDMAChannel::_2),
            static_cast<uint8_t>(BluePillDMAChannel::_3),
            static_cast<uint8_t>(BluePillDMAChannel::_4),
            static_cast<uint8_t>(BluePillDMAChannel::_5),
            static_cast<uint8_t>(BluePillDMAChannel::_6),
            static_cast<uint8_t>(BluePillDMAChannel::_7)
        };
        const uint32_t dma;
};