#pragma once

#include <cstdint>
#include <span>

#include "../utils.h"

class II2C {
public:
    virtual ~II2C() = default;

    [[nodiscard]] virtual utils::ErrorCode write(uint8_t addr, std::span<const uint8_t> data) const = 0;
    [[nodiscard]] virtual utils::ErrorCode read(uint8_t addr, std::span<uint8_t> data) const = 0;
};
