#include "DMA.h"
#include "USART.h"
#include "doctest.h"
#include <cstdint>
#include <vector>

#include "test_events.h"
#include <algorithm>

template <typename T> static T get_le(const std::vector<uint8_t>& data)
{
    T res = 0;
    for (size_t i = 0; i < sizeof(T); i++) {
        if (data.size() > i) {
            res |= data[i] << (i * 8);
        }
    }
    return res;
}

TEST_CASE("DMA setup and enable/disable recordings")
{
    test_event_clear();

    DMA dma(BluePillDMAController::_1, RCC_DMA1);

    // Setup a channel in PER2MEM mode
    dma.setup_channel(DMADirection::PER2MEM, BluePillDMAChannel::_1, 0x40000000, BluePillDMAPeripheralWordSize::BYTE,
        0x20000000, BluePillDMAMemWordSize::BYTE, BluePillDMAPriority::HIGH, 8, true, true, true, true, true, false);

    auto events = test_event_get_all();

    // Verify DMAPeriphAddr
    auto p_it = std::find_if(
        events.begin(), events.end(), [](const TestEvent& e) { return e.type == TestEventType::DMAPeriphAddr; });
    REQUIRE(p_it != events.end());
    CHECK(p_it->id == static_cast<uint32_t>(BluePillDMAChannel::_1));
    CHECK(get_le<uint32_t>(p_it->data) == 0x40000000u);

    // Verify DMAMemAddr
    auto m_it = std::find_if(
        events.begin(), events.end(), [](const TestEvent& e) { return e.type == TestEventType::DMAMemAddr; });
    REQUIRE(m_it != events.end());
    CHECK(m_it->id == static_cast<uint32_t>(BluePillDMAChannel::_1));
    CHECK(get_le<uint32_t>(m_it->data) == 0x20000000u);

    // Verify DMANumber
    auto n_it = std::find_if(
        events.begin(), events.end(), [](const TestEvent& e) { return e.type == TestEventType::DMANumber; });
    REQUIRE(n_it != events.end());
    CHECK(n_it->id == static_cast<uint32_t>(BluePillDMAChannel::_1));
    CHECK(get_le<uint16_t>(n_it->data) == 8u);

    // Enable the channel
    dma.enable(BluePillDMAChannel::_1);

    events = test_event_get_all();
    auto e_it = std::find_if(
        events.begin(), events.end(), [](const TestEvent& e) { return e.type == TestEventType::DMAEnable; });
    CHECK(e_it != events.end());

    // Disable and reset
    dma.disable(BluePillDMAChannel::_1);

    events = test_event_get_all();
    auto d_it = std::find_if(
        events.begin(), events.end(), [](const TestEvent& e) { return e.type == TestEventType::DMADisable; });
    CHECK(d_it != events.end());

    dma.reset(BluePillDMAChannel::_1);

    CHECK(true);
}

TEST_CASE("USARTWithDMA get_dma_count reads DMA CNDTR")
{
    test_event_clear();

    // Set the DMA count register
    test_set_dma_cndtr(1234);

    static volatile std::atomic_bool rx_err_flag(false);
    static volatile std::atomic_bool rx_half_flag(false);
    static volatile std::atomic_bool rx_complete_flag(false);

    static volatile std::atomic_bool tx_err_flag(false);
    static volatile std::atomic_bool tx_half_flag(false);
    static volatile std::atomic_bool tx_complete_flag(false);

    static volatile std::atomic_bool overrun_error_flag(false);
    static volatile std::atomic_bool tx_transfer_complete_flag(false);

    DMA dma(BluePillDMAController::_1, RCC_DMA1);

    DMAChannelAndFlags rx_chan { BluePillDMAChannel::_2, &rx_err_flag, &rx_half_flag, &rx_complete_flag };
    DMAChannelAndFlags tx_chan { BluePillDMAChannel::_3, &tx_err_flag, &tx_half_flag, &tx_complete_flag };
    USARTDMA udma { &dma, rx_chan, tx_chan };

    USARTWithDMA usart_dma(
        BluePillUSART::_2, RCC_USART2, RST_USART2, &overrun_error_flag, &tx_transfer_complete_flag, udma);

    CHECK(usart_dma.get_dma_count() == 1234);
}
