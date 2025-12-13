#pragma once

#include <cstddef>
#include <span>

class ISerial {
public:
    virtual ~ISerial() = default;

    // Send data in blocking mode
    virtual void send_blocking(std::span<const std::byte> data) const = 0;
    void send_blocking(std::byte b) const { send_blocking(std::span<const std::byte>(&b, 1)); }
};
