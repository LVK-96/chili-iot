#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/dma.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/usart.h>

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
    (void)addr;
    (void)w;
    (void)wlen;
    (void)r;
    (void)rlen;
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
}
uint16_t usart_recv_blocking(uint32_t usart)
{
    (void)usart;
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
volatile uint32_t mock_dma_cndtr;
void dma_channel_reset(uint32_t dma, uint8_t channel)
{
    (void)dma;
    (void)channel;
}
void dma_set_peripheral_address(uint32_t dma, uint8_t channel, uint32_t address)
{
    (void)dma;
    (void)channel;
    (void)address;
}
void dma_set_memory_address(uint32_t dma, uint8_t channel, uint32_t address)
{
    (void)dma;
    (void)channel;
    (void)address;
}
void dma_set_number_of_data(uint32_t dma, uint8_t channel, uint16_t number)
{
    (void)dma;
    (void)channel;
    (void)number;
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
}
void dma_disable_channel(uint32_t dma, uint8_t channel)
{
    (void)dma;
    (void)channel;
}

volatile uint32_t mock_dma_ccr;
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

// NVIC Mocks
void nvic_enable_irq(uint8_t irqn) { (void)irqn; }
void nvic_set_priority(uint8_t irqn, uint8_t priority)
{
    (void)irqn;
    (void)priority;
}
