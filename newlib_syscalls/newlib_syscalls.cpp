#include "Logger.h"
#include "System.h"

extern "C" {

// Just to make printf print to USART for now

int _write([[maybe_unused]] int file, char* ptr, int len)
{
    if (_system::peripherals::usart1.get_is_setup()) {
        for (int i = 0; i < len; ++i) {
            _system::peripherals::usart1.send_blocking(*ptr++);
        }
        return len;
    }

    return -1;
}
}