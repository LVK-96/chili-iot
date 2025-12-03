#include "DMA.h"
#include "USART.h"
#include "doctest.h"
#include "mock_libopencm3.h"

#include <array>

TEST_CASE("DMA setup and enable/disable recordings")
{
    mock_libopencm3_reset();

    DMA dma(BluePillDMAController::_1, RCC_DMA1);

    // Setup a channel in PER2MEM mode
    dma.setup_channel(DMADirection::PER2MEM, BluePillDMAChannel::_1, 0x40000000, BluePillDMAPeripheralWordSize::BYTE,
        0x20000000, BluePillDMAMemWordSize::BYTE, BluePillDMAPriority::HIGH, 8, true, true, true, true, true);

    // Verify that the peripheral/memory addresses and number of data were recorded
    REQUIRE(mock_dma_periph_addr_calls.size() >= 1);
    REQUIRE(mock_dma_mem_addr_calls.size() >= 1);
    REQUIRE(mock_dma_number_calls.size() >= 1);

    CHECK(mock_dma_periph_addr_calls.back().address == 0x40000000u);
    CHECK(mock_dma_mem_addr_calls.back().address == 0x20000000u);
    CHECK(mock_dma_number_calls.back().number == 8u);

    // Enable the channel and check enable count
    dma.enable(BluePillDMAChannel::_1);
    CHECK(mock_dma_enable_channel_count >= 1);

    // Disable and reset
    dma.disable(BluePillDMAChannel::_1);
    CHECK(mock_dma_disable_channel_count >= 1);

    dma.reset(BluePillDMAChannel::_1);

    CHECK(true);
}

TEST_CASE("USARTWithDMA get_dma_count reads DMA CNDTR")
{
    mock_libopencm3_reset();

    // Set the mocked DMA count register
    mock_dma_cndtr = 1234;

    static volatile std::atomic_bool rx_err_flag(false);
    static volatile std::atomic_bool rx_half_flag(false);
    static volatile std::atomic_bool rx_complete_flag(false);

    static volatile std::atomic_bool tx_err_flag(false);
    static volatile std::atomic_bool tx_half_flag(false);
    static volatile std::atomic_bool tx_complete_flag(false);

    DMA dma(BluePillDMAController::_1, RCC_DMA1);

    DMAChannelAndFlags rx_chan { BluePillDMAChannel::_2, &rx_err_flag, &rx_half_flag, &rx_complete_flag };
    DMAChannelAndFlags tx_chan { BluePillDMAChannel::_3, &tx_err_flag, &tx_half_flag, &tx_complete_flag };
    USARTDMA udma { &dma, rx_chan, tx_chan };

    USARTWithDMA usart_dma(BluePillUSART::_2, RCC_USART2, RST_USART2, udma);

    CHECK(usart_dma.get_dma_count() == static_cast<unsigned int>(mock_dma_cndtr));
}
