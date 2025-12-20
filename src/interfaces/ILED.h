#pragma once

class ILED {
public:
    constexpr ILED() noexcept = default;
    virtual void toggle() const = 0;
    virtual void on() const = 0;
    virtual void off() const = 0;
};