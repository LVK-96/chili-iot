#include <algorithm>
#include <array>
#include <functional>

#include <libopencm3/cm3/nvic.h>

#include "BlinkyLED.h"
#include "DMA.h"
#include "GPIO.h"
#include "I2C.h"
#include "Logger.h"
#include "Network.h"
#include "System.h"
#include "Temperature.h"
#include "USART.h"

namespace sensor_node_system {

namespace peripherals {
    GPIOPort gpio_a { BluePillGPIOPort::A, RCC_GPIOA, RST_GPIOA };
    GPIOPort gpio_b { BluePillGPIOPort::B, RCC_GPIOB, RST_GPIOB };
    GPIOPort gpio_c { BluePillGPIOPort::C, RCC_GPIOC, RST_GPIOC };
    GPIOPin led_pin { LED_PIN_NRO, gpio_c };
    DMA dma1 { BluePillDMAController::_1, RCC_DMA1 }; // DMA peripherals don't have reset bits in RCC CSRs
    USART usart1 { BluePillUSART::_1, RCC_USART1, RST_USART1 };
    USARTWithDMA usart2 { BluePillUSART::_2, RCC_USART2, RST_USART2,
        { .dma = dma1, .rx_channel = BluePillDMAChannel::_6, .tx_channel = BluePillDMAChannel::_7 } };
    I2C i2c1 { BluePillI2C::_1, RCC_I2C1, RST_I2C1 };
}

namespace modules {
    GPIOLED led { peripherals::led_pin };
    USARTLogger logger { Logger::LogLevel::INFO, peripherals::usart1 };
    BME280TemperatureSensor temperature { logger, peripherals::i2c1,
        BME280I2CBusAddr::SECONDARY }; // The Waveshare BME280 module defaults to the secondary I2C address (0x77)
    ESP8266Network network { logger, peripherals::usart2 };
}

void nop(unsigned int n)
{
    // NOP n times
    for (unsigned int i = 0; i < n; ++i) {
        __asm__("nop");
    }
}

static void peripheral_setup()
{
    // GPIO
    for (const auto& gpio : { peripherals::gpio_a, peripherals::gpio_b, peripherals::gpio_c }) {
        gpio.reset_pulse();
    }
    peripherals::gpio_a.setup_pins(LOGGER_TX_PIN_NRO | NETWORK_TX_PIN_NRO, GPIOMode::OUTPUT_50_MHZ,
        GPIOFunction::OUTPUT_ALTFN_PUSHPULL); // A9 USART1 & A2 USART2 TX
    peripherals::gpio_a.setup_pins(NETWORK_RX_PIN_NRO, GPIOMode::INPUT, GPIOFunction::INPUT_FLOAT); // A3 USART2 RX
    peripherals::gpio_b.setup_pins(I2C1_SCL_PIN_NRO | I2C1_SDA_PIN_NRO, GPIOMode::OUTPUT_50_MHZ,
        GPIOFunction::OUTPUT_ALTFN_OPENDRAIN); // B6 SCL, B7 SDA
    peripherals::gpio_c.setup_pins(LED_PIN_NRO, GPIOMode::OUTPUT_2_MHZ, GPIOFunction::OUTPUT_PUSHPULL); // C13 LED

    // USART
    auto usart_setup_helper = [](USART& usart, unsigned int baudrate, unsigned int databits, USARTStopBits stopbits,
                                  USARTMode mode, USARTParity parity, USARTFlowControl flowcontrol) {
        usart.reset_pulse();
        usart.disable();
        usart.setup(baudrate, databits, stopbits, mode, parity, flowcontrol);
        usart.enable();
    };
    usart_setup_helper(peripherals::usart1, LOGGER_BAUDRATE, LOGGER_DATABITS, USARTStopBits::_1, USARTMode::TX,
        USARTParity::NONE, USARTFlowControl::NONE);
    usart_setup_helper(peripherals::usart2, NETWORK_BAUDRATE, NETWORK_DATABITS, USARTStopBits::_1, USARTMode::TX_RX,
        USARTParity::NONE, USARTFlowControl::NONE);

    // DMA
    peripherals::dma1.disable();
    peripherals::dma1.enable();

    // I2C
    peripherals::gpio_b.set_pins(I2C1_SCL_PIN_NRO | I2C1_SDA_PIN_NRO); // I2C Idle high
    peripherals::i2c1.reset_pulse();
    peripherals::i2c1.disable();
    peripherals::i2c1.setup();
    peripherals::i2c1.enable();
}

static void clock_setup()
{
    rcc_clock_setup_pll(&rcc_hse_configs[RCC_CLOCK_HSE8_72MHZ]);
    systick_set_clocksource(STK_CSR_CLKSOURCE_AHB_DIV8); // 72MHz / 8 -> 9MHz, 9000000 counts per second
    peripherals::gpio_a.clk_enable();
    peripherals::gpio_b.clk_enable();
    peripherals::gpio_c.clk_enable();
    peripherals::dma1.clk_enable();
    peripherals::usart1.clk_enable();
    peripherals::usart2.clk_enable();
    peripherals::i2c1.clk_enable();
}

static void systick_setup()
{
    systick_set_reload(SYSTICK_RELOAD_VALUE);
    systick_counter_enable();
}

static void interrupt_setup()
{
    nvic_enable_irq(NVIC_DMA1_CHANNEL6_IRQ); // DMA1 Channel 6, USART2 RX uses this channel
    nvic_enable_irq(NVIC_USART2_IRQ); // USART2 interrupts
    systick_interrupt_enable();
}

void temperature_setup()
{
    while (modules::temperature.init() != ErrorCode::OK) {
        ;
    }
}

void network_setup()
{
    do {
        modules::network.reset();
    } while (modules::network.connect_to_ap() != ErrorCode::OK);
}

void setup()
{
    clock_setup();

    // After the peripherals are setup the logger is ready
    peripheral_setup();
    modules::logger.info("Starting sensor node...\n");
    modules::logger.info("Peripherals setup!\n");

    interrupt_setup();
    modules::logger.info("Interrupts setup!\n");

    systick_setup();
    modules::logger.info("Systick counter setup!\n");

    modules::logger.info("Setting up temperature sensor...\n");
    temperature_setup();
    modules::logger.info("Temperature sensor setup!\n");

    modules::logger.info("Setting up network connection...\n");
    network_setup();
    modules::logger.info("Network connection setup!\n");
}

uint32_t systick() { return systick_counter; }

static uint32_t systick_delta(uint32_t start, uint32_t end)
{
    uint32_t diff = 0;
    if (end > start) {
        diff = end - start;
    } else {
        diff = UINT32_MAX - (start - end) + 1;
    }

    return diff;
}

void sleep(uint32_t ticks)
{
    unsigned int slept_ticks = 0;
    const unsigned int start = systick();
    do {
        slept_ticks = systick_delta(start, systick());
    } while (slept_ticks < ticks);
}

void sleep_ms(unsigned int ms)
{
    static constexpr unsigned int ticks_in_ms = 1;
    sleep(ms * ticks_in_ms);
}

}
