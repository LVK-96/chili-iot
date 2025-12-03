#include "DMA.h"
#include "USART.h"
#include "doctest.h"
#include "mock_libopencm3.h"

#include <array>
#include <atomic>

TEST_CASE("USART basic operations")
{
    mock_libopencm3_reset();

    USART usart(BluePillUSART::_2, RCC_USART2, RST_USART2);

    // Setup should mark the peripheral as configured
    usart.setup(115200, 8, USARTStopBits::_1, USARTMode::TX_RX, USARTParity::NONE, USARTFlowControl::NONE);
    CHECK(usart.get_is_setup());

    // Sending and receiving (mocked functions increment counters)
    usart.send_blocking(static_cast<uint8_t>(0x5A));
    std::array<uint8_t, 3> bytes = { 'H', 'i', '!' };
    usart.send_blocking(std::span<uint8_t>(bytes.data(), bytes.size()));
    usart.send_blocking('A');
    usart.send_blocking(std::string_view("hello"));

    CHECK(usart.recieve_blocking() == 0);

    // The mock should have recorded sends: 1 + 3 + 1 + 5 = 10
    CHECK(static_cast<int>(mock_usart_send_bytes.size()) == 10);
    CHECK(mock_usart_recv_blocking_count == 1);

    // Test interrupts toggles (no counters for interrupts, just ensure no crash)
    usart.rx_interrupt(true);
    usart.tx_interrupt(true);
    usart.error_interrupt(true);
    usart.rx_interrupt(false);
    usart.tx_interrupt(false);
    usart.error_interrupt(false);
}

TEST_CASE("USARTWithDMA basic DMA operations")
{
    mock_libopencm3_reset();

    DMA dma(BluePillDMAController::_1, RCC_DMA1);

    // Create volatile atomic flags as required by the USART DMA helper struct
    static volatile std::atomic_bool rx_err_flag(false);
    static volatile std::atomic_bool rx_half_flag(false);
    static volatile std::atomic_bool rx_complete_flag(false);

    static volatile std::atomic_bool tx_err_flag(false);
    static volatile std::atomic_bool tx_half_flag(false);
    static volatile std::atomic_bool tx_complete_flag(false);

    DMAChannelAndFlags rx_chan { BluePillDMAChannel::_2, &rx_err_flag, &rx_half_flag, &rx_complete_flag };
    DMAChannelAndFlags tx_chan { BluePillDMAChannel::_3, &tx_err_flag, &tx_half_flag, &tx_complete_flag };

    USARTDMA udma { &dma, rx_chan, tx_chan };

    USARTWithDMA usart_dma(BluePillUSART::_2, RCC_USART2, RST_USART2, udma);

    // Prepare a dummy buffer and enable DMA for RX
    std::array<uint8_t, 8> buff = {};
    usart_dma.enable_rx_dma(
        static_cast<uint32_t>(reinterpret_cast<uintptr_t>(buff.data())), static_cast<unsigned int>(buff.size()));

    // The mock should have recorded DMA channel enable
    CHECK(mock_dma_enable_channel_count >= 1);

    // Reset and disable
    usart_dma.reset_rx_dma();
    usart_dma.disable_rx_dma();

    CHECK(mock_dma_disable_channel_count >= 1);

    // get_dma_count uses mocked DMA backend; just ensure call is valid
    (void)usart_dma.get_dma_count();

    CHECK(true);
}
