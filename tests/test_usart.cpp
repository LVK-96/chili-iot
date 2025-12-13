#include "DMA.h"
#include "USART.h"
#include "doctest.h"

#include <array>
#include <atomic> // Keep this as std::atomic_bool is still used in the second test case
#include <cstdint>
#include <string>
#include <vector>

#include "test_events.h"
#include <algorithm>

TEST_CASE("USART basic operations")
{
    test_event_clear();

    static volatile std::atomic_bool overrun_error_flag(false);
    static volatile std::atomic_bool tx_transfer_complete_flag(false);
    USART usart2(BluePillUSART::_2, RCC_USART2, RST_USART2, &overrun_error_flag, &tx_transfer_complete_flag);

    usart2.setup(115200, 8, USARTStopBits::_1, USARTMode::TX_RX, USARTParity::NONE, USARTFlowControl::NONE);
    usart2.enable();

    // Send generic bytes
    std::string msg = "abc";
    usart2.send_blocking(std::as_bytes(std::span(msg)));

    // Send single byte
    std::byte d { 'd' };
    usart2.send_blocking(d);

    // Check for UsartTx event
    auto events = test_event_get_all();
    auto tx_it = std::find_if(
        events.begin(), events.end(), [](const TestEvent& e) { return e.type == TestEventType::UsartTx; });
    REQUIRE(tx_it != events.end());

    CHECK(tx_it->data.size() == 4);
    CHECK(tx_it->data[0] == 'a');
    CHECK(tx_it->data[3] == 'd');

    // Receive
    // The backend always returns 0 for recv but increments count
    auto val = usart2.recieve_blocking();
    CHECK(val == 0);

    events = test_event_get_all();
    auto rx_it = std::find_if(
        events.begin(), events.end(), [](const TestEvent& e) { return e.type == TestEventType::UsartRxCount; });
    REQUIRE(rx_it != events.end());
    // count is 1, so 4 bytes in data (uint32_t)
    CHECK(rx_it->data.size() == 4);
    CHECK(rx_it->data[0] == 1);
}

TEST_CASE("USARTWithDMA basic DMA operations")
{
    test_event_clear();

    DMA dma(BluePillDMAController::_1, RCC_DMA1);

    static volatile std::atomic_bool rx_err_flag(false);

    // Create volatile atomic flags as required by the USART DMA helper struct
    static volatile std::atomic_bool rx_half_flag(false);
    static volatile std::atomic_bool rx_complete_flag(false);

    static volatile std::atomic_bool tx_err_flag(false);
    static volatile std::atomic_bool tx_half_flag(false);
    static volatile std::atomic_bool tx_complete_flag(false);

    static volatile std::atomic_bool overrun_error_flag(false);
    static volatile std::atomic_bool tx_transfer_complete_flag(false);

    DMAChannelAndFlags rx_chan { BluePillDMAChannel::_2, &rx_err_flag, &rx_half_flag, &rx_complete_flag };
    DMAChannelAndFlags tx_chan { BluePillDMAChannel::_3, &tx_err_flag, &tx_half_flag, &tx_complete_flag };

    USARTDMA udma { &dma, rx_chan, tx_chan };

    USARTWithDMA usart_dma(
        BluePillUSART::_2, RCC_USART2, RST_USART2, &overrun_error_flag, &tx_transfer_complete_flag, udma);

    // Prepare a dummy buffer and enable DMA for RX
    std::array<uint8_t, 8> buff = {};
    usart_dma.enable_rx_dma(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(buff.data())),
        static_cast<unsigned int>(buff.size()), true, true, true, false);

    // Check for DMAEnable
    auto events = test_event_get_all();
    auto e_it = std::find_if(
        events.begin(), events.end(), [](const TestEvent& e) { return e.type == TestEventType::DMAEnable; });
    CHECK(e_it != events.end());

    // Reset and disable
    usart_dma.reset_rx_dma();
    usart_dma.disable_rx_dma();

    // Check for DMADisable
    events = test_event_get_all();
    auto d_it = std::find_if(
        events.begin(), events.end(), [](const TestEvent& e) { return e.type == TestEventType::DMADisable; });
    CHECK(d_it != events.end());

    // get_dma_count uses underlying DMA backend; just ensure call is valid
    (void)usart_dma.get_dma_count();

    CHECK(true);
}
