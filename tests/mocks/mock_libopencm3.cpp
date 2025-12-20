extern "C" {
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/dma.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/usart.h>
}

#include "mock_libopencm3.h"
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <vector>

template <typename T, typename U> static void push_bounded(std::vector<T>& vec, U&& value, size_t limit = 1024)
{
    if (vec.size() >= limit) {
        vec.clear();
        vec.shrink_to_fit();
    }
    vec.push_back(std::forward<U>(value));
}

// --- Shared Trial Vectors (C++ types, cannot be extern "C") ---

std::vector<MockI2CCall> mock_i2c_calls;
std::vector<uint16_t> mock_usart_send_bytes;
std::vector<MockDMANumberCall> mock_dma_number_calls;
std::vector<MockDMAAddressCall> mock_dma_periph_addr_calls;
std::vector<MockDMAAddressCall> mock_dma_mem_addr_calls;

// --- Shared POD Variables (extern "C" linkage) ---
extern "C" {
int mock_usart_recv_blocking_count = 0;
int mock_dma_enable_channel_count = 0;
int mock_dma_disable_channel_count = 0;
uint32_t mock_dma_cndtr = 0;

volatile uint32_t mock_usart_dr = 0;
volatile uint32_t mock_usart_sr = 0x80; // TXE by default
volatile uint32_t mock_usart_cr3 = 0;
volatile uint32_t mock_i2c_cr1 = 0;
volatile uint32_t mock_dma_ccr = 0;

uint32_t rcc_apb1_frequency = 36000000;
const struct rcc_clock_scale rcc_hse_configs[] = { { .pll_mul = 9,
    .pll_source = 1,
    .hpre = 0,
    .ppre1 = 4,
    .ppre2 = 0,
    .adcpre = 0,
    .flash_waitstates = 2,
    .prediv1 = 0,
    .prediv1_source = 0,
    .prediv2 = 0,
    .pll2_mul = 0,
    .pll3_mul = 0,
    .usbpre = 0,
    .ahb_frequency = 72000000,
    .apb1_frequency = 36000000,
    .apb2_frequency = 72000000 } };

void mock_libopencm3_reset()
{
    mock_i2c_calls.clear();
    mock_usart_send_bytes.clear();
    mock_usart_recv_blocking_count = 0;
    mock_dma_number_calls.clear();
    mock_dma_periph_addr_calls.clear();
    mock_dma_mem_addr_calls.clear();
    mock_dma_enable_channel_count = 0;
    mock_dma_disable_channel_count = 0;
    mock_dma_cndtr = 0;
    mock_usart_sr = 0x80;
}

// --- Mock Implementations (Available for both Host and QEMU for now) ---
void gpio_set_mode(uint32_t gpioport, uint8_t mode, uint8_t cnf, uint16_t gpios)
{
    (void)gpioport;
    (void)mode;
    (void)cnf;
    (void)gpios;
}
void gpio_set(uint32_t gpioport, uint16_t gpios)
{
    (void)gpioport;
    (void)gpios;
}
void gpio_clear(uint32_t gpioport, uint16_t gpios)
{
    (void)gpioport;
    (void)gpios;
}
void gpio_toggle(uint32_t gpioport, uint16_t gpios)
{
    (void)gpioport;
    (void)gpios;
}
uint16_t gpio_get(uint32_t gpioport, uint16_t gpios)
{
    (void)gpioport;
    (void)gpios;
    return 0;
}

void rcc_periph_clock_enable(enum rcc_periph_clken clken) { (void)clken; }
void rcc_periph_clock_disable(enum rcc_periph_clken clken) { (void)clken; }
void rcc_periph_reset_pulse(enum rcc_periph_rst rst) { (void)rst; }
void rcc_periph_reset_hold(enum rcc_periph_rst rst) { (void)rst; }
void rcc_periph_reset_release(enum rcc_periph_rst rst) { (void)rst; }
void rcc_clock_setup_pll(const struct rcc_clock_scale* clock) { (void)clock; }

void systick_set_clocksource(uint8_t clocksource) { (void)clocksource; }
void systick_set_reload(uint32_t reload) { (void)reload; }
void systick_clear() { }
void systick_counter_enable() { }
void systick_interrupt_enable() { }

void nvic_enable_irq(uint8_t irqn) { (void)irqn; }
void nvic_set_priority(uint8_t irqn, uint8_t priority)
{
    (void)irqn;
    (void)priority;
}

// I2C Mocks
void i2c_peripheral_disable(uint32_t i2c) { (void)i2c; }
void i2c_peripheral_enable(uint32_t i2c) { (void)i2c; }
void i2c_set_speed(uint32_t i2c, enum i2c_speeds speed, uint32_t clock)
{
    (void)i2c;
    (void)speed;
    (void)clock;
}

void i2c_reset(uint32_t i2c) { (void)i2c; }
void i2c_send_start(uint32_t i2c) { (void)i2c; }
void i2c_send_stop(uint32_t i2c) { (void)i2c; }
void i2c_send_data(uint32_t i2c, uint8_t data)
{
    (void)i2c;
    (void)data;
}
uint8_t i2c_get_data(uint32_t i2c)
{
    (void)i2c;
    return 0;
}
void i2c_enable_ack(uint32_t i2c) { (void)i2c; }
void i2c_disable_ack(uint32_t i2c) { (void)i2c; }
void i2c_set_own_7bit_slave_address(uint32_t i2c, uint8_t slave)
{
    (void)i2c;
    (void)slave;
}

void i2c_transfer7(uint32_t i2c, uint8_t addr, const uint8_t* w, size_t wlen, uint8_t* r, size_t rlen)
{
    MockI2CCall call;
    call.addr = addr;
    call.wlen = wlen;
    call.rlen = rlen;
    if (w && wlen > 0)
        call.wdata.assign(w, w + wlen);

    push_bounded(mock_i2c_calls, std::move(call));

    if (r && rlen > 0) {
        // Generic mock: return 0s for all reads
        std::memset(r, 0, rlen);
    }
}

// USART Mocks
void usart_set_baudrate(uint32_t usart, uint32_t baud)
{
    (void)usart;
    (void)baud;
}
void usart_set_databits(uint32_t usart, uint32_t bits)
{
    (void)usart;
    (void)bits;
}
void usart_set_stopbits(uint32_t usart, uint32_t stopbits)
{
    (void)usart;
    (void)stopbits;
}
void usart_set_mode(uint32_t usart, uint32_t mode)
{
    (void)usart;
    (void)mode;
}
void usart_set_parity(uint32_t usart, uint32_t parity)
{
    (void)usart;
    (void)parity;
}
void usart_set_flow_control(uint32_t usart, uint32_t flowcontrol)
{
    (void)usart;
    (void)flowcontrol;
}
void usart_enable(uint32_t usart) { (void)usart; }
void usart_disable(uint32_t usart) { (void)usart; }
void usart_send_blocking(uint32_t usart, uint16_t data)
{
    (void)usart;
    push_bounded(mock_usart_send_bytes, data);
}
uint16_t usart_recv_blocking(uint32_t usart)
{
    (void)usart;
    mock_usart_recv_blocking_count++;
    return 0;
}
void usart_enable_rx_interrupt(uint32_t usart) { (void)usart; }
void usart_disable_rx_interrupt(uint32_t usart) { (void)usart; }
void usart_enable_tx_interrupt(uint32_t usart) { (void)usart; }
void usart_disable_tx_interrupt(uint32_t usart) { (void)usart; }
void usart_enable_tx_complete_interrupt(uint32_t usart) { (void)usart; }
void usart_disable_tx_complete_interrupt(uint32_t usart) { (void)usart; }
void usart_enable_error_interrupt(uint32_t usart) { (void)usart; }
void usart_disable_error_interrupt(uint32_t usart) { (void)usart; }
void usart_enable_idle_interrupt(uint32_t usart) { (void)usart; }
void usart_disable_idle_interrupt(uint32_t usart) { (void)usart; }

bool usart_get_flag(uint32_t usart, uint32_t flag)
{
    (void)usart;
    (void)flag;
    return false;
}
void usart_send(uint32_t usart, uint16_t data)
{
    (void)usart;
    push_bounded(mock_usart_send_bytes, data);
}
uint16_t usart_recv(uint32_t usart)
{
    (void)usart;
    return 0;
}
void usart_enable_rx_dma(uint32_t usart) { (void)usart; }
void usart_disable_rx_dma(uint32_t usart) { (void)usart; }
void usart_enable_tx_dma(uint32_t usart) { (void)usart; }
void usart_disable_tx_dma(uint32_t usart) { (void)usart; }

// DMA Mocks
void dma_channel_reset(uint32_t dma, uint8_t channel)
{
    (void)dma;
    (void)channel;
}
void dma_set_peripheral_address(uint32_t dma, uint8_t channel, uint32_t address)
{
    (void)dma;
    push_bounded(mock_dma_periph_addr_calls, MockDMAAddressCall { channel, address });
}
void dma_set_memory_address(uint32_t dma, uint8_t channel, uint32_t address)
{
    (void)dma;
    push_bounded(mock_dma_mem_addr_calls, MockDMAAddressCall { channel, address });
}
void dma_set_number_of_data(uint32_t dma, uint8_t channel, uint16_t number)
{
    (void)dma;
    push_bounded(mock_dma_number_calls, MockDMANumberCall { channel, number });
}
void dma_set_read_from_peripheral(uint32_t dma, uint8_t channel)
{
    (void)dma;
    (void)channel;
}
void dma_set_read_from_memory(uint32_t dma, uint8_t channel)
{
    (void)dma;
    (void)channel;
}
void dma_enable_memory_increment_mode(uint32_t dma, uint8_t channel)
{
    (void)dma;
    (void)channel;
}
void dma_disable_memory_increment_mode(uint32_t dma, uint8_t channel)
{
    (void)dma;
    (void)channel;
}
void dma_enable_peripheral_increment_mode(uint32_t dma, uint8_t channel)
{
    (void)dma;
    (void)channel;
}
void dma_disable_peripheral_increment_mode(uint32_t dma, uint8_t channel)
{
    (void)dma;
    (void)channel;
}
void dma_enable_circular_mode(uint32_t dma, uint8_t channel)
{
    (void)dma;
    (void)channel;
}
void dma_enable_mem2mem_mode(uint32_t dma, uint8_t channel)
{
    (void)dma;
    (void)channel;
}
void dma_set_peripheral_size(uint32_t dma, uint8_t channel, uint32_t size)
{
    (void)dma;
    (void)channel;
    (void)size;
}
void dma_set_memory_size(uint32_t dma, uint8_t channel, uint32_t size)
{
    (void)dma;
    (void)channel;
    (void)size;
}
void dma_set_priority(uint32_t dma, uint8_t channel, uint32_t priority)
{
    (void)dma;
    (void)channel;
    (void)priority;
}
void dma_enable_transfer_complete_interrupt(uint32_t dma, uint8_t channel)
{
    (void)dma;
    (void)channel;
}
void dma_disable_transfer_complete_interrupt(uint32_t dma, uint8_t channel)
{
    (void)dma;
    (void)channel;
}
void dma_enable_transfer_error_interrupt(uint32_t dma, uint8_t channel)
{
    (void)dma;
    (void)channel;
}
void dma_disable_transfer_error_interrupt(uint32_t dma, uint8_t channel)
{
    (void)dma;
    (void)channel;
}
void dma_enable_half_transfer_interrupt(uint32_t dma, uint8_t channel)
{
    (void)dma;
    (void)channel;
}
void dma_disable_half_transfer_interrupt(uint32_t dma, uint8_t channel)
{
    (void)dma;
    (void)channel;
}
void dma_enable_channel(uint32_t dma, uint8_t channel)
{
    (void)dma;
    (void)channel;
    mock_dma_enable_channel_count++;
}
void dma_disable_channel(uint32_t dma, uint8_t channel)
{
    (void)dma;
    (void)channel;
    mock_dma_disable_channel_count++;
}
bool dma_get_interrupt_flag(uint32_t dma, uint8_t channel, uint32_t flag)
{
    (void)dma;
    (void)channel;
    (void)flag;
    return false;
}
void dma_clear_interrupt_flags(uint32_t dma, uint8_t channel, uint32_t flags)
{
    (void)dma;
    (void)channel;
    (void)flags;
}

uint16_t dma_get_number_of_data(uint32_t dma, uint8_t channel)
{
    (void)dma;
    (void)channel;
    return (uint16_t)mock_dma_cndtr;
}

void test_set_dma_cndtr(uint32_t value) { mock_dma_cndtr = value; }

} // extern "C"
