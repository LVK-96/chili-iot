#pragma once

#include <cstdint>

#define NVIC_USART1_IRQ 37
#define NVIC_USART2_IRQ 38
#define NVIC_DMA1_CHANNEL4_IRQ 14
#define NVIC_DMA1_CHANNEL5_IRQ 15
#define NVIC_DMA1_CHANNEL6_IRQ 16
#define NVIC_DMA1_CHANNEL7_IRQ 17

#ifdef __cplusplus
extern "C" {
#endif
void nvic_enable_irq(uint8_t irqn);
void nvic_set_priority(uint8_t irqn, uint8_t priority);
#ifdef __cplusplus
}
#endif
