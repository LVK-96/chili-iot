#include <cstdint>
#include <cstdio>

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/usart.h>

#include "GPIO.h"
#include "Logger.h"
#include "System.h"
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

void Logger::error(std::string_view msg, sensor_node_system::ErrorCode code) const
{
    log("ERROR", LogLevel::ERROR);

    constexpr unsigned int code_string_length = 7;
    char code_buf[code_string_length + 1];
    snprintf(code_buf, code_string_length, " (%hhu)", static_cast<uint8_t>(code));
    log(code_buf, LogLevel::ERROR);

    log(": ", LogLevel::ERROR);
    log(msg, LogLevel::ERROR);
}

sensor_node_system::ErrorCode USARTLogger::_log(std::string_view msg) const
{
    if (usart.get_is_setup()) {
        for (const auto& ch : msg) {
            usart.send_blocking(ch);
        }
        return sensor_node_system::ErrorCode::OK;
    }

    return sensor_node_system::ErrorCode::USART_NOT_SETUP_ERROR;
}