#pragma once

#include <cstdarg>
#include <cstdio>
#include <string_view>

#include "USART.h"
#include "utils.h"

class Logger {
public:
    enum class LogLevel { INFO = 0, WARNING = 1, ERROR = 2, SILENT = 3 };

    constexpr Logger(LogLevel verbosity) noexcept
        : verbosity(verbosity)
    {
    }
    void set_verbosity(LogLevel verbosity);
    [[nodiscard]] LogLevel get_verbosity() const;

    // Simple log with string view
    void log(std::string_view msg) const;

    // Printf-style interface
    void log(LogLevel level, const char* fmt, ...) const;
    void info(const char* fmt, ...) const;
    void warning(const char* fmt, ...) const;
    void error(const char* fmt, ...) const;

    // Internal printf helper
    void _log_vprintf(LogLevel level, const char* fmt, va_list args) const;

protected:
    LogLevel verbosity;
    virtual void _log(std::string_view msg) const = 0;
};

class USARTLogger final : public Logger {
public:
    constexpr USARTLogger(LogLevel verbosity, const ISerial* usart) noexcept
        : Logger(verbosity)
        , usart(usart)
    {
    }

private:
    const ISerial* usart;
    void _log(std::string_view msg) const override;
};