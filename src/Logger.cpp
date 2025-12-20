#include <cstdio>

#include "Logger.h"

void Logger::set_verbosity(LogLevel new_verbosity) { verbosity = new_verbosity; }

Logger::LogLevel Logger::get_verbosity() const { return verbosity; }

void Logger::log(std::string_view msg) const { _log(msg); }

void Logger::_log_vprintf(LogLevel level, const char* fmt, va_list args) const
{
    if (level >= verbosity) {
        constexpr unsigned int buffer_size = 128;
        char buffer[buffer_size];

        // Add log level prefix
        const char* level_str = "";
        switch (level) {
        case LogLevel::INFO:
            level_str = "INFO: ";
            break;
        case LogLevel::WARNING:
            level_str = "WARNING: ";
            break;
        case LogLevel::ERROR:
            level_str = "ERROR: ";
            break;
        case LogLevel::SILENT:
            level_str = "";
            break;
        }

        int written = snprintf(buffer, buffer_size, "%s", level_str);
        if (written > 0 && written < (int)buffer_size) {
            vsnprintf(buffer + written, buffer_size - written, fmt, args);
        }
        _log(std::string_view(buffer));
    }
}

void Logger::log(LogLevel level, const char* fmt, ...) const
{
    va_list args;
    va_start(args, fmt);
    _log_vprintf(level, fmt, args);
    va_end(args);
}

void Logger::info(const char* fmt, ...) const
{
    va_list args;
    va_start(args, fmt);
    _log_vprintf(LogLevel::INFO, fmt, args);
    va_end(args);
}

void Logger::warning(const char* fmt, ...) const
{
    va_list args;
    va_start(args, fmt);
    _log_vprintf(LogLevel::WARNING, fmt, args);
    va_end(args);
}

void Logger::error(const char* fmt, ...) const
{
    va_list args;
    va_start(args, fmt);
    _log_vprintf(LogLevel::ERROR, fmt, args);
    va_end(args);
}

void Logger::_log(std::string_view msg) const { std::fwrite(msg.data(), 1, msg.size(), stdout); }