#pragma once

#include <string_view>

#include <libopencm3/stm32/usart.h>

#include "GPIO.h"
#include "System.h"
#include "USART.h"

class Logger {
public:
    enum class LogLevel { INFO = 0, WARNING = 1, ERROR = 2, SILENT = 3 };

    constexpr Logger(LogLevel verbosity) noexcept
        : verbosity(verbosity)
    {
    }
    void set_verbosity(LogLevel verbosity);
    LogLevel get_verbosity() const;

    void log(std::string_view msg, LogLevel level = LogLevel::INFO) const;

    void info(std::string_view msg) const;
    void warning(std::string_view msg) const;
    void error(std::string_view msg) const;
    void error(std::string_view msg, _system::ErrorCode code) const;

protected:
    LogLevel verbosity;
    virtual _system::ErrorCode _log(std::string_view msg) const = 0;
};

class USARTLogger : public Logger {
public:
    constexpr USARTLogger(LogLevel verbosity, const USART& usart) noexcept
        : Logger(verbosity)
        , usart(usart)
    {
    }

private:
    const USART& usart;
    _system::ErrorCode _log(std::string_view msg) const override;
};