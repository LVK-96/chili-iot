#include "utils.h"
#include "Logger.h"
#include "System.h"

Logger utils::logger { Logger::LogLevel::INFO };

void utils::nop(unsigned int n)
{
    // NOP n times
    for (unsigned int i = 0; i < n; ++i) {
        __asm__("nop");
    }
}