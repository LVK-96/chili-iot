#include "SensorNode.h"
#include "System.h"

int main()
{
    auto setup_res = _system::setup();
    if (setup_res == _system::ErrorCode::OK) {
        _system::modules::logger.info("System setup OK!\n");
        SensorNode(_system::modules::led, _system::modules::logger, _system::modules::temperature).main_loop();
    }

    _system::modules::logger.error("System setup error!\n", setup_res);

    while (true) {
        _system::nop(1);
    }

    return -1; // This will never be reached
}