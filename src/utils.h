#pragma once

#include <cstdint>

template <unsigned int bits> constexpr inline uint32_t mask32()
{
    static_assert(bits <= 32);
    return ((1 << bits) - (uint32_t)1);
}

template <unsigned int bits> constexpr inline uint16_t mask16()
{
    static_assert(bits <= 16);
    return ((1 << bits) - (uint16_t)1);
}

template <unsigned int bits> constexpr inline uint8_t mask8()
{
    static_assert(bits <= 8);
    return ((1 << bits) - (uint8_t)1);
}