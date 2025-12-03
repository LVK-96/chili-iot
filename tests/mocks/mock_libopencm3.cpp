#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/dma.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/usart.h>

#include "mock_libopencm3.h"
#include <algorithm>
#include <bme280_defs.h>
#include <cstdint>
#include <unordered_map>
#include <vector>

// Mock implementations
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

void systick_set_clocksource(uint8_t clocksource) { (void)clocksource; }

void systick_set_reload(uint32_t reload) { (void)reload; }

void systick_clear() { }

void systick_counter_enable() { }

void systick_interrupt_enable() { }

// I2C Mocks
void i2c_peripheral_disable(uint32_t i2c) { (void)i2c; }
void i2c_peripheral_enable(uint32_t i2c) { (void)i2c; }
void i2c_set_speed(uint32_t i2c, uint32_t speed, uint32_t clock)
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
uint32_t i2c_sr1(uint32_t i2c)
{
    (void)i2c;
    return 0;
}
uint32_t i2c_sr2(uint32_t i2c)
{
    (void)i2c;
    return 0;
}

volatile uint32_t mock_i2c_cr1;
void i2c_set_own_7bit_slave_address(uint32_t i2c, uint8_t slave)
{
    (void)i2c;
    (void)slave;
}
void i2c_transfer7(uint32_t i2c, uint8_t addr, const uint8_t* w, size_t wlen, uint8_t* r, size_t rlen)
{
    (void)i2c;
    MockI2CCall call;
    call.addr = addr;
    call.wlen = wlen;
    call.rlen = rlen;

    const bool has_write = w && wlen;
    if (has_write) {
        call.wdata.assign(w, w + wlen);
    }
    mock_i2c_calls.push_back(std::move(call));

    // Minimal BME280 emulation state per I2C addr
    struct BMEState {
        uint8_t last_reg = 0;
        bool resetting = false;
        int reset_ticks = 0; // ticks remaining where STATUS.IM_UPDATE remains set
        uint8_t ctrl_meas = 0;
    };

    static std::unordered_map<uint8_t, BMEState> bme_states;

    // If this is a write, store the last register or handle register writes
    if (has_write) {
        auto& state = bme_states[addr];
        // If writing more than one byte, first byte is reg addr
        uint8_t reg = w[0];
        state.last_reg = reg;
        if (wlen >= 2) {
            // If writing reset command
            if (reg == BME280_REG_RESET && w[1] == BME280_SOFT_RESET_COMMAND) {
                state.resetting = true;
                state.reset_ticks = 2; // allow a couple of status polls
            }
            // If writing ctrl_meas or ctrl hum, record for completeness
            if (reg == BME280_REG_CTRL_HUM || reg == BME280_REG_CTRL_MEAS) {
                state.ctrl_meas = w[1];
            }
        }
    }

    const bool has_read = r && rlen;
    if (has_read) {
        auto& state = bme_states[addr];

        // If last register is known, return appropriate data
        switch (state.last_reg) {
        case BME280_REG_CHIP_ID:
            if (rlen >= 1) {
                r[0] = BME280_CHIP_ID;
                if (rlen > 1)
                    std::fill_n(r + 1, rlen - 1, 0);
            }
            break;
        case BME280_REG_STATUS:
            if (rlen >= 1) {
                if (state.resetting && state.reset_ticks > 0) {
                    r[0] = BME280_STATUS_IM_UPDATE;
                    state.reset_ticks--;
                    if (state.reset_ticks == 0)
                        state.resetting = false;
                } else {
                    r[0] = 0;
                }
            }
            break;
        case BME280_REG_TEMP_PRESS_CALIB_DATA: {
            // Provide calibration data (26 bytes). First six bytes set
            // so dig_t1=27504, dig_t2=26435, dig_t3=-1000 which
            // together with the raw ADC below yield ~21.0°C.
            const uint8_t calib[26] = { 0x70, 0x6B, 0x43, 0x67, 0x18, 0xFC, 0x22, 0x11, 0x10, 0x20, 0x30, 0x40, 0x50,
                0x60, 0x70, 0x80, 0x90, 0xA0, 0xB0, 0xC0, 0xD0, 0xE0, 0xF0, 0x01, 0x02, 0x03 };
            size_t copy = (rlen < sizeof(calib)) ? rlen : sizeof(calib);
            std::copy(calib, calib + copy, r);
            if (rlen > copy)
                std::fill_n(r + copy, rlen - copy, 0);
        } break;
        case BME280_REG_HUMIDITY_CALIB_DATA: {
            // Provide plausible humidity calibration (7 bytes)
            const uint8_t hcalib[7] = { 0x01, 0x80, 0x02, 0x90, 0x03, 0xA0, 0x04 };
            size_t copy = (rlen < sizeof(hcalib)) ? rlen : sizeof(hcalib);
            std::copy(hcalib, hcalib + copy, r);
            if (rlen > copy)
                std::fill_n(r + copy, rlen - copy, 0);
        } break;
        case BME280_REG_DATA: {
            // Return raw pressure(3), temp(3), hum(2) bytes (8 total).
            // Temperature raw chosen as 0x7B,0xBF,0x00 -> raw ADC 506864
            // which with the calibration above produces ~21.0°C.
            const uint8_t data[8] = {
                0x6A, 0xBC, 0x00, // pressure msb, lsb, xlsb (unchanged)
                0x7B, 0xBF, 0x00, // temp msb, lsb, xlsb -> raw temp ~506864
                0x40, 0x00 // humidity msb, lsb
            };
            size_t copy = (rlen < sizeof(data)) ? rlen : sizeof(data);
            std::copy(data, data + copy, r);
            if (rlen > copy)
                std::fill_n(r + copy, rlen - copy, 0);
        } break;
        default:
            // Unrecognized register: return zeros
            std::fill_n(r, rlen, 0);
            break;
        }
    }
}

uint32_t rcc_apb1_frequency = 36000000;

const struct rcc_clock_scale rcc_hse_configs[] = { { 0, 0, 0, 0, 0, 0, 72000000, 36000000, 72000000 } };

void rcc_clock_setup_pll(const struct rcc_clock_scale* clock) { (void)clock; }

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
    (void)data;
    mock_usart_send_bytes.push_back(static_cast<uint16_t>(data & 0xff));
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
void usart_enable_error_interrupt(uint32_t usart) { (void)usart; }
void usart_disable_error_interrupt(uint32_t usart) { (void)usart; }
uint32_t usart_get_flag(uint32_t usart, uint32_t flag)
{
    (void)usart;
    (void)flag;
    return 0;
}
void usart_send(uint32_t usart, uint16_t data)
{
    (void)usart;
    (void)data;
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

volatile uint32_t mock_usart_dr;
volatile uint32_t mock_usart_sr;
volatile uint32_t mock_usart_cr3;

// DMA Mocks
uint32_t mock_dma_cndtr;
void dma_channel_reset(uint32_t dma, uint8_t channel)
{
    (void)dma;
    (void)channel;
}
void dma_set_peripheral_address(uint32_t dma, uint8_t channel, uint32_t address)
{
    (void)dma;
    mock_dma_periph_addr_calls.push_back({ channel, address });
}
void dma_set_memory_address(uint32_t dma, uint8_t channel, uint32_t address)
{
    (void)dma;
    mock_dma_mem_addr_calls.push_back({ channel, address });
}
void dma_set_number_of_data(uint32_t dma, uint8_t channel, uint16_t number)
{
    (void)dma;
    mock_dma_number_calls.push_back({ channel, number });
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

uint32_t mock_dma_ccr;
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

// Detailed log containers
std::vector<MockI2CCall> mock_i2c_calls;
std::vector<uint16_t> mock_usart_send_bytes;
int mock_usart_recv_blocking_count = 0;

std::vector<MockDMANumberCall> mock_dma_number_calls;
std::vector<MockDMAAddressCall> mock_dma_periph_addr_calls;
std::vector<MockDMAAddressCall> mock_dma_mem_addr_calls;
int mock_dma_enable_channel_count = 0;
int mock_dma_disable_channel_count = 0;

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
}

// NVIC Mocks
void nvic_enable_irq(uint8_t irqn) { (void)irqn; }
void nvic_set_priority(uint8_t irqn, uint8_t priority)
{
    (void)irqn;
    (void)priority;
}
