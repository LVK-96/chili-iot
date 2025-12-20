#pragma once

#include <optional>

#include "interfaces/ITemperatureSensor.h"
#include "utils.h"

class MockTemperatureSensor final : public ITemperatureSensor {
public:
    utils::ErrorCode init() const override { return utils::ErrorCode::OK; }
    std::optional<double> read() const override { return 20.0; }
};