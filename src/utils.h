#pragma once

#include <cstdint>

class USARTLogger;

namespace utils {

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshift-count-overflow"
template <unsigned int bits> constexpr inline uint32_t mask32()
{
    static_assert(bits <= 32);
    return ((((uint32_t)1 << bits) - (uint32_t)1));
}

template <unsigned int bits> constexpr inline uint16_t mask16()
{
    static_assert(bits <= 16);
    return (((uint16_t)1 << bits) - (uint16_t)1);
}

template <unsigned int bits> constexpr inline uint8_t mask8()
{
    static_assert(bits <= 8);
    return (((uint8_t)1 << bits) - (uint8_t)1); //
}
#pragma GCC diagnostic pop

enum class ErrorCode : uint8_t {
    OK = 0,
    TEMPERATURE_INIT_ERROR = 1,
    USART_NOT_SETUP_ERROR = 10,
    NETWORK_RESPONSE_NOT_OK_ERROR = 20,
    NETWORK_RESPONSE_OVERRUN_ERROR = 21,
    NETWORK_RESPONSE_TIMEOUT_ERROR = 22,
    NETWORK_RESPONSE_DMA_ERROR = 23,
    UNEXPECTED_ERROR = 255
};

extern USARTLogger logger;

void nop(unsigned int);

}; // namespace utils