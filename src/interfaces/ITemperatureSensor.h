#pragma once

#include <optional>

#include "utils.h"

class ITemperatureSensor {
public:
    [[nodiscard]] virtual utils::ErrorCode init() const = 0;
    [[nodiscard]] virtual std::optional<double> read() const = 0;
};