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

#ifdef __cplusplus
extern "C" {
#endif
void rcc_periph_clock_enable(enum rcc_periph_clken clken);
void rcc_periph_clock_disable(enum rcc_periph_clken clken);
void rcc_periph_reset_pulse(enum rcc_periph_rst rst);
void rcc_periph_reset_hold(enum rcc_periph_rst rst);
void rcc_periph_reset_release(enum rcc_periph_rst rst);
#ifdef __cplusplus
}
#endif

extern uint32_t rcc_apb1_frequency;

struct rcc_clock_scale {
    uint8_t pll_mul;
    uint8_t pll_source;
    uint8_t hpre;
    uint8_t ppre1;
    uint8_t ppre2;
    uint8_t adcpre;
    uint8_t flash_waitstates;
    uint8_t prediv1;
    uint8_t prediv1_source;
    uint8_t prediv2;
    uint8_t pll2_mul;
    uint8_t pll3_mul;
    uint8_t usbpre;
    uint32_t ahb_frequency;
    uint32_t apb1_frequency;
    uint32_t apb2_frequency;
};

extern const struct rcc_clock_scale rcc_hse_configs[];
#define RCC_CLOCK_HSE8_72MHZ 0

#ifdef __cplusplus
extern "C" {
#endif
void rcc_clock_setup_pll(const struct rcc_clock_scale* clock);
#ifdef __cplusplus
}
#endif
