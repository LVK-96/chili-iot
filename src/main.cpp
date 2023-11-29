#include "SensorNode.h"
#include "System.h"


int main()
{
    auto setup_res = sensor_node_system::setup();
    if (setup_res == sensor_node_system::ErrorCode::OK) {
        sensor_node_system::modules::logger.info("System setup OK!\n");
        SensorNode(sensor_node_system::modules::led, sensor_node_system::modules::logger, sensor_node_system::modules::temperature).main_loop();
    }

    sensor_node_system::modules::logger.error("System setup error!\n", setup_res);

    while (true) {
        sensor_node_system::nop(1);
    }

    return -1; // This will never be reached
}