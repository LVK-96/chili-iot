#include "System.h"
#include "USART.h"

extern "C" {

int _write([[maybe_unused]] int file, char *ptr, int len)
{
    if (sensor_node_system::peripherals::usart1.get_is_setup()) {
        for (int i = 0; i < len; ++i)
        {
            sensor_node_system::peripherals::usart1.send_blocking(*ptr++);
        }
        return len;
    }

    return -1;
}

}