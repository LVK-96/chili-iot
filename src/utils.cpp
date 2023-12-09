#include "Logger.h"
#include "System.h"

USARTLogger utils::logger { Logger::LogLevel::INFO, &bluepill::peripherals::usart1 };

void utils::nop(unsigned int n)
{
    // NOP n times
    for (unsigned int i = 0; i < n; ++i) {
        __asm__("nop");
    }
}