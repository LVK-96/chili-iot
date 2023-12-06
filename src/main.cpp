#include "SensorNode.h"
#include "System.h"

int main()
{
    sensor_node_system::setup();
    sensor_node_system::modules::logger.info("System setup OK!\n");

    SensorNode(sensor_node_system::modules::led, sensor_node_system::modules::logger,
        sensor_node_system::modules::temperature, sensor_node_system::modules::network)
        .main_loop();

    return -1; // This will never be reached
}