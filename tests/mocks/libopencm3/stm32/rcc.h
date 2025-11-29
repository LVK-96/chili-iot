#pragma once

#include <cstdint>

enum rcc_periph_clken {
    RCC_GPIOA,
    RCC_GPIOB,
    RCC_GPIOC,
    RCC_USART1,
    RCC_USART2,
    RCC_I2C1,
    RCC_DMA1,
};

enum rcc_periph_rst {
    RST_GPIOA,
    RST_GPIOB,
    RST_GPIOC,
    RST_USART1,
    RST_USART2,
    RST_I2C1,
    RST_DMA1,
};

void rcc_periph_clock_enable(enum rcc_periph_clken clken);
void rcc_periph_clock_disable(enum rcc_periph_clken clken);
void rcc_periph_reset_pulse(enum rcc_periph_rst rst);
void rcc_periph_reset_hold(enum rcc_periph_rst rst);
void rcc_periph_reset_release(enum rcc_periph_rst rst);

extern uint32_t rcc_apb1_frequency;

struct rcc_clock_scale {
    uint32_t pll;
    uint32_t pllsrc;
    uint32_t hpre;
    uint32_t ppre1;
    uint32_t ppre2;
    uint32_t flash_waitstates;
    uint32_t ahb_frequency;
    uint32_t apb1_frequency;
    uint32_t apb2_frequency;
};

extern const struct rcc_clock_scale rcc_hse_configs[];
#define RCC_CLOCK_HSE8_72MHZ 0

void rcc_clock_setup_pll(const struct rcc_clock_scale* clock);
