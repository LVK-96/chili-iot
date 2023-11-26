#include <algorithm>

#include "BlinkyLED.h"
#include "GPIO.h"
#include "I2C.h"
#include "Logger.h"
#include "System.h"
#include "Temperature.h"
#include "USART.h"

namespace _system {

namespace peripherals {
    GPIOPort gpio_a { BluePillGPIOPort::A, RCC_GPIOA, RST_GPIOA };
    GPIOPort gpio_b { BluePillGPIOPort::B, RCC_GPIOB, RST_GPIOB };
    GPIOPort gpio_c { BluePillGPIOPort::C, RCC_GPIOC, RST_GPIOC };
    GPIOPin led_pin { LED_PIN_NRO, gpio_c };
    USART usart1 { BluePillUSART::_1, RCC_USART1, RST_USART1 };
    USART usart2 { BluePillUSART::_2, RCC_USART2, RST_USART2 };
    I2C i2c1 { BluePillI2C::_1, RCC_I2C1, RST_I2C1 };
}

namespace modules {
    GPIOLED led { peripherals::led_pin };
    USARTLogger logger { Logger::LogLevel::INFO, peripherals::usart1 };
    BME280TemperatureSensor temperature { logger, peripherals::i2c1, BME280I2CBusAddr::SECONDARY }; // The Waveshare BME280 module defaults to the secondary I2C address (0x77)
}

void nop(unsigned int n)
{
    // NOP n times
    for (unsigned int i = 0; i < n; ++i) {
        __asm__("nop");
    }
}

void peripheral_setup()
{
    // GPIO A: USART
    peripherals::gpio_a.clk_enable();
    peripherals::gpio_a.setup_pins(LOGGER_TX_PIN_NRO | NETWORK_TX_PIN_NRO | NETWORK_RX_PIN_NRO, GPIOMode::OUTPUT_50_MHZ, GPIOFunction::OUTPUT_ALTFN_PUSHPULL);

    // GPIO B: I2C
    peripherals::gpio_b.clk_enable();
    peripherals::gpio_b.setup_pins(I2C1_SCL_PIN_NRO | I2C1_SDA_PIN_NRO, GPIOMode::OUTPUT_50_MHZ, GPIOFunction::OUTPUT_ALTFN_OPENDRAIN);

    // GPIOC: LED
    peripherals::gpio_c.clk_enable();
    peripherals::gpio_c.setup_pins(LED_PIN_NRO, GPIOMode::OUTPUT_2_MHZ, GPIOFunction::OUTPUT_PUSHPULL);

    // USART
    peripherals::usart1.clken_reset_disable_setup_enable(LOGGER_BAUDRATE, LOGGER_DATABITS, USARTStopBits::_1, USARTMode::TX, USARTParity::NONE, USARTFlowControl::NONE);
    peripherals::usart2.clken_reset_disable_setup_enable(NETWORK_BAUDRATE, NETWORK_DATABITS, USARTStopBits::_1, USARTMode::TX_RX, USARTParity::NONE, USARTFlowControl::NONE);

    // I2C
    peripherals::i2c1.clk_enable();
    peripherals::gpio_b.set_pins(I2C1_SCL_PIN_NRO | I2C1_SDA_PIN_NRO); // Idle high
    peripherals::i2c1.reset_pulse();
    peripherals::i2c1.disable();
    peripherals::i2c1.setup();
    peripherals::i2c1.enable();
}

static void clock_setup()
{
    rcc_clock_setup_pll(&rcc_hse_configs[RCC_CLOCK_HSE8_72MHZ]);
    systick_set_clocksource(STK_CSR_CLKSOURCE_AHB_DIV8); // 72MHz / 8 -> 9MHz, 9000000 counts per second
}

static void systick_setup()
{
    systick_set_reload(SYSTICK_RELOAD_VALUE);
    systick_interrupt_disable();
    systick_counter_enable();
}

static ErrorCode module_setup()
{
    return modules::temperature.init();
}

ErrorCode setup()
{
    clock_setup();
    systick_setup();
    peripheral_setup();
    return module_setup();
}

uint32_t systick() { return systick_get_value(); }

uint32_t diff_ticks(uint32_t older, uint32_t newer)
{
    // Systicks count down
    if (older >= newer) {
        return (older - newer);
    } else {
        // newer has wrapped around and is bigger than older
        //-> count distance from older to SYSTICK_RELOAD_VALUE and then
        // FROM SYSTICK_RELOAD_VALUE to newer
        return older + 1 + (SYSTICK_RELOAD_VALUE - newer);
    }
}

void sleep(uint32_t ticks)
{
    while (ticks > 0) {
        uint32_t sleep_ticks = std::min(ticks, SYSTICK_RELOAD_VALUE);
        uint32_t t0 = systick();
        while (diff_ticks(t0, systick()) < sleep_ticks)
            ;
        ticks -= sleep_ticks;
    }
}

void sleep_us(uint32_t us)
{
    constexpr unsigned int ticks_in_us = SYSTICK_CLOCK_MHZ;
    sleep(ticks_in_us * us);
}

void sleep_ms(uint32_t ms)
{
    constexpr unsigned int ticks_in_ms = (SYSTICK_CLOCK_MHZ)*1e3;
    sleep(ticks_in_ms * ms);
}

}