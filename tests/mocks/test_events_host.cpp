#include "mock_freertos.h"
#include "mock_libopencm3.h"
#include "test_events.h"

#include <type_traits>

template <typename T> static void push_le(std::vector<uint8_t>& data, T value)
{
    using U = std::make_unsigned_t<T>;
    auto uv = static_cast<U>(value);
    for (size_t i = 0; i < sizeof(T); ++i) {
        data.push_back(static_cast<uint8_t>(uv & 0xFF));
        uv >>= 8;
    }
}

void test_event_record(TestEventType type, uint32_t id, const void* data, size_t len)
{
    // Host-side tests don't need runtime recording; tests should inspect mocks.
    (void)type;
    (void)id;
    (void)data;
    (void)len;
}

std::vector<TestEvent> test_event_get_all()
{
    std::vector<TestEvent> out;

    // Queue sends (include ticks_to_wait as first 4 bytes)
    for (const auto& s : mock_queue_send_calls) {
        TestEvent ev;
        ev.type = TestEventType::QueueSend;
        ev.id = s.queue;
        push_le(ev.data, s.ticks_to_wait);
        ev.data.insert(ev.data.end(), s.data.begin(), s.data.end());
        out.push_back(std::move(ev));
    }

    // Queue receives
    for (const auto& r : mock_queue_receive_calls) {
        TestEvent ev;
        ev.type = TestEventType::QueueReceive;
        ev.id = r.queue;
        ev.data = r.data;
        out.push_back(std::move(ev));
    }

    // Task creations
    for (const auto& t : mock_task_create_calls) {
        TestEvent ev;
        ev.type = TestEventType::TaskCreate;
        ev.id = 0;
        push_le(ev.data, t.stack_depth);
        ev.data.push_back(static_cast<uint8_t>(t.priority & 0xFF));
        // name length + name bytes
        ev.data.push_back(static_cast<uint8_t>(t.name.size() & 0xFF));
        for (char c : t.name)
            ev.data.push_back(static_cast<uint8_t>(c));
        out.push_back(std::move(ev));
    }

    // I2C transfers
    for (const auto& c : mock_i2c_calls) {
        TestEvent ev;
        ev.type = TestEventType::I2CTransfer;
        ev.id = c.addr;
        // layout: [wlen, rlen, ...wdata]
        ev.data.push_back(static_cast<uint8_t>(c.wlen & 0xFF));
        ev.data.push_back(static_cast<uint8_t>(c.rlen & 0xFF));
        for (auto b : c.wdata)
            ev.data.push_back(b);
        out.push_back(std::move(ev));
    }

    // USART TX: combine bytes into single event if present
    if (!mock_usart_send_bytes.empty()) {
        TestEvent ev;
        ev.type = TestEventType::UsartTx;
        ev.id = 0;
        ev.data.reserve(mock_usart_send_bytes.size());
        for (auto b : mock_usart_send_bytes)
            ev.data.push_back(static_cast<uint8_t>(b));
        out.push_back(std::move(ev));
    }

    // USART receive blocking count
    if (mock_usart_recv_blocking_count > 0) {
        TestEvent ev;
        ev.type = TestEventType::UsartRxCount;
        ev.id = 0;
        push_le<uint32_t>(ev.data, mock_usart_recv_blocking_count);
        out.push_back(std::move(ev));
    }

    // DMA peripheral address calls
    for (const auto& p : mock_dma_periph_addr_calls) {
        TestEvent ev;
        ev.type = TestEventType::DMAPeriphAddr;
        ev.id = p.channel;
        push_le(ev.data, p.address);
        out.push_back(std::move(ev));
    }

    // DMA memory address calls
    for (const auto& m : mock_dma_mem_addr_calls) {
        TestEvent ev;
        ev.type = TestEventType::DMAMemAddr;
        ev.id = m.channel;
        push_le(ev.data, m.address);
        out.push_back(std::move(ev));
    }

    // DMA number calls
    for (const auto& n : mock_dma_number_calls) {
        TestEvent ev;
        ev.type = TestEventType::DMANumber;
        ev.id = n.channel;
        push_le(ev.data, n.number);
        out.push_back(std::move(ev));
    }

    // DMA enable/disable counts as single events (if non-zero)
    if (mock_dma_enable_channel_count > 0) {
        TestEvent ev;
        ev.type = TestEventType::DMAEnable;
        ev.id = 0;
        push_le<uint32_t>(ev.data, mock_dma_enable_channel_count);
        out.push_back(std::move(ev));
    }
    if (mock_dma_disable_channel_count > 0) {
        TestEvent ev;
        ev.type = TestEventType::DMADisable;
        ev.id = 0;
        push_le<uint32_t>(ev.data, mock_dma_disable_channel_count);
        out.push_back(std::move(ev));
    }

    return out;
}

std::optional<TestEvent> test_event_get_last(TestEventType type)
{
    auto all = test_event_get_all();
    for (auto it = all.rbegin(); it != all.rend(); ++it) {
        if (it->type == type)
            return *it;
    }
    return std::nullopt;
}

void test_event_clear()
{
    // Clearing host-side events means clearing the underlying mocks
    mock_libopencm3_reset();
    mock_freertos_reset();
}

void test_set_dma_cndtr(uint32_t value) { mock_dma_cndtr = value; }
