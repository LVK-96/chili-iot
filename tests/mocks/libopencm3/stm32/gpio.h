#pragma once

#include <cstdint>

#define GPIOA 0x40010800
#define GPIOB 0x40010C00
#define GPIOC 0x40011000

#define GPIO_MODE_INPUT 0x0
#define GPIO_MODE_OUTPUT_10_MHZ 0x1
#define GPIO_MODE_OUTPUT_2_MHZ 0x2
#define GPIO_MODE_OUTPUT_50_MHZ 0x3

#define GPIO_CNF_INPUT_ANALOG 0x0
#define GPIO_CNF_INPUT_FLOAT 0x1
#define GPIO_CNF_INPUT_PULL_UPDOWN 0x2
#define GPIO_CNF_OUTPUT_PUSHPULL 0x0
#define GPIO_CNF_OUTPUT_OPENDRAIN 0x1
#define GPIO_CNF_OUTPUT_ALTFN_PUSHPULL 0x2
#define GPIO_CNF_OUTPUT_ALTFN_OPENDRAIN 0x3

#define GPIO0 (1 << 0)
#define GPIO1 (1 << 1)
#define GPIO2 (1 << 2)
#define GPIO3 (1 << 3)
#define GPIO4 (1 << 4)
#define GPIO5 (1 << 5)
#define GPIO6 (1 << 6)
#define GPIO7 (1 << 7)
#define GPIO8 (1 << 8)
#define GPIO9 (1 << 9)
#define GPIO10 (1 << 10)
#define GPIO11 (1 << 11)
#define GPIO12 (1 << 12)
#define GPIO13 (1 << 13)
#define GPIO14 (1 << 14)
#define GPIO15 (1 << 15)

#define GPIO_USART1_TX GPIO9
#define GPIO_USART1_RX GPIO10
#define GPIO_USART2_TX GPIO2
#define GPIO_USART2_RX GPIO3
#define GPIO_I2C1_SCL GPIO6
#define GPIO_I2C1_SDA GPIO7

void gpio_set_mode(uint32_t gpioport, uint8_t mode, uint8_t cnf, uint16_t gpios);
void gpio_set(uint32_t gpioport, uint16_t gpios);
void gpio_clear(uint32_t gpioport, uint16_t gpios);
void gpio_toggle(uint32_t gpioport, uint16_t gpios);
uint16_t gpio_get(uint32_t gpioport, uint16_t gpios);
