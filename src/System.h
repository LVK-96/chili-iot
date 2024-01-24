#pragma once

#include <cstdint>

#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>

#include "utils.h"

// Forward declarations
class GPIOPort;
struct GPIOPin;
class USART;
class USARTWithDMA;
class I2C;
class DMA;

namespace bluepill {

enum class ErrorCode : uint8_t {
    OK = 0,
    TEMPERATURE_INIT_ERROR = 1,
    USART_NOT_SETUP_ERROR = 10,
    NETWORK_RESPONSE_NOT_OK_ERROR = 20,
    NETWORK_RESPONSE_OVERRUN_ERROR = 21,
    UNEXPECTED_ERROR = 255
};

void setup();

// Time functions
uint32_t systick();
void sleep(uint32_t ticks);
void busy_wait_ms(unsigned int ms);
void async_wait(uint32_t ticks);
void async_wait_ms(unsigned int ms);

// Time related constants
// These are actually programmed into the CSRs in
// clock_setup with libopencm3: rcc_clock_setup_pll(&rcc_hse_configs[RCC_CLOCK_HSE8_72MHZ])
constexpr unsigned int AHB_CLOCK_MHZ = 72;
constexpr unsigned int APB1_CLOCK_MHZ = 36;
constexpr unsigned int APB2_CLOCK_MHZ = 72;
// libopencm3: systick_set_clocksource(STK_CSR_CLKSOURCE_AHB_DIV8)
constexpr unsigned int SYSTICK_CLOCK_MHZ = AHB_CLOCK_MHZ / 8; // 9 MHz
constexpr unsigned int SYSTICK_BITS = 24;
constexpr uint32_t SYSTICK_RELOAD_VALUE = 8999; // tick every 1ms

// What pins are used
constexpr unsigned int LED_PIN_NRO = GPIO13;
constexpr unsigned int ESP_RESET_PIN_NRO = GPIO15;
constexpr unsigned int LOGGER_TX_PIN_NRO = GPIO_USART1_TX;
constexpr unsigned int NETWORK_TX_PIN_NRO = GPIO_USART2_TX;
constexpr unsigned int NETWORK_RX_PIN_NRO = GPIO_USART2_RX;
constexpr unsigned int I2C1_SCL_PIN_NRO = GPIO_I2C1_SCL;
constexpr unsigned int I2C1_SDA_PIN_NRO = GPIO_I2C1_SDA;

// USART parameters
constexpr unsigned int LOGGER_BAUDRATE = 38400;
constexpr unsigned int LOGGER_DATABITS = 8;
constexpr unsigned int NETWORK_BAUDRATE = 115200;
constexpr unsigned int NETWORK_DATABITS = 8;

namespace peripherals {
    extern GPIOPort gpio_a;
    extern GPIOPort gpio_b;
    extern GPIOPort gpio_c;
    extern GPIOPin led_pin;
    extern GPIOPin esp_reset_pin;
    extern USART usart1;
    extern USARTWithDMA usart2;
    extern I2C i2c1;
    extern DMA dma1;
};

}; // namespace bluepill