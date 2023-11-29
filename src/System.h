#pragma once

#include <cstdint>

#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>

#include "utils.h"

// Forward declarations
class GPIOPort;
class GPIOPin;
class USART;
class I2C;
class GPIOLED;
class USARTLogger;
class BME280TemperatureSensor;

namespace sensor_node_system {

enum class ErrorCode : uint8_t {
    OK = 0,
    TEMPERATURE_INIT_ERROR = 1,
    USART_NOT_SETUP_ERROR = 10,
    UNEXPECTED_ERROR = 255
};

void nop(unsigned int n);
ErrorCode setup();

// Time functions
uint32_t systick();
uint32_t diff_ticks(uint32_t older, uint32_t newer);
void sleep(uint32_t ticks);
void sleep_us(uint32_t us);
void sleep_ms(uint32_t ms);

// Time related constants
// These are actually programmed into the CSRs in
// clock_setup with libopencm3: rcc_clock_setup_pll(&rcc_hse_configs[RCC_CLOCK_HSE8_72MHZ])
constexpr unsigned int AHB_CLOCK_MHZ = 72;
constexpr unsigned int APB1_CLOCK_MHZ = 36;
constexpr unsigned int APB2_CLOCK_MHZ = 72;
// libopencm3: systick_set_clocksource(STK_CSR_CLKSOURCE_AHB_DIV8)
constexpr unsigned int SYSTICK_CLOCK_MHZ = AHB_CLOCK_MHZ / 8; // 9 MHz
constexpr unsigned int SYSTICK_BITS = 24;
constexpr uint32_t SYSTICK_RELOAD_VALUE = UINT32_MAX & mask32<SYSTICK_BITS>();

// What pins are used
constexpr unsigned int LED_PIN_NRO = GPIO13;
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
    extern USART usart1;
    extern USART usart2;
    extern I2C i2c1;
};

namespace modules {
    extern GPIOLED led;
    extern USARTLogger logger;
    extern BME280TemperatureSensor temperature;
};

};