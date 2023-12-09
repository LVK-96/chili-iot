#include "System.h"
#include "USART.h"

extern "C" {

int _write([[maybe_unused]] int file, char* ptr, int len)
{
    bluepill::peripherals::usart1.send_blocking(std::string_view { ptr, static_cast<size_t>(len) });
    return len;
}
}
