#include <cstdint>
#include <cstdio>

#include "Logger.h"
#include "USART.h"

void Logger::set_verbosity(LogLevel new_verbosity) { verbosity = new_verbosity; }

Logger::LogLevel Logger::get_verbosity() const { return verbosity; }

void Logger::log(std::string_view msg, LogLevel level) const
{
    if (level >= verbosity) {
        _log(msg);
    }
}

void Logger::info(std::string_view msg) const
{
    log("INFO: ", LogLevel::INFO);
    log(msg, LogLevel::INFO);
}

void Logger::warning(std::string_view msg) const
{
    log("WARNING: ", LogLevel::WARNING);
    log(msg, LogLevel::WARNING);
}

void Logger::error(std::string_view msg) const
{
    log("ERROR: ", LogLevel::ERROR);
    log(msg, LogLevel::ERROR);
}

void Logger::error(std::string_view msg, utils::ErrorCode code) const
{
    log("ERROR", LogLevel::ERROR);

    constexpr unsigned int code_string_length = 7;
    char code_buf[code_string_length + 1];
    snprintf(code_buf, code_string_length, " (%hhu)", static_cast<uint8_t>(code));
    log(code_buf, LogLevel::ERROR);

    log(": ", LogLevel::ERROR);
    log(msg, LogLevel::ERROR);
}

void USARTLogger::_log(std::string_view msg) const
{
    for (const auto& ch : msg) {
        usart->send_blocking(ch);
    }
}