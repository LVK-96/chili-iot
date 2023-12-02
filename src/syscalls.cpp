#include "System.h"
#include "USART.h"

extern "C" {

int _write([[maybe_unused]] int file, char* ptr, int len)
{
    if (sensor_node_system::peripherals::usart1.get_is_setup()) {
        sensor_node_system::peripherals::usart1.send_blocking(std::string_view { ptr, static_cast<size_t>(len) });
        return len;
    }

    return -1;
}
}
