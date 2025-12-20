#pragma once

#include <cstdint>

#define STK_CSR_CLKSOURCE_AHB_DIV8 0

#ifdef __cplusplus
extern "C" {
#endif
void systick_set_clocksource(uint8_t clocksource);
void systick_set_reload(uint32_t reload);
void systick_clear();
void systick_counter_enable();
void systick_interrupt_enable();
#ifdef __cplusplus
}
#endif
