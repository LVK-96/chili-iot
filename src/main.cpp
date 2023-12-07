#include "SensorNode.h"
#include "System.h"

int main()
{
    sensor_node_system::setup();
    sensor_node_system::logger.info("System setup OK!\n");

    GPIOLED led { &sensor_node_system::peripherals::led_pin };
    BME280TemperatureSensor temperature { &sensor_node_system::logger, &sensor_node_system::peripherals::i2c1,
        BME280I2CBusAddr::SECONDARY }; // The Waveshare BME280 module defaults to the secondary I2C address (0x77)
    ESP8266Network network { &sensor_node_system::logger, &sensor_node_system::peripherals::usart2,
        &sensor_node_system::peripherals::esp_reset_pin };

    network.hard_reset(); // ESP8266 seems to be sometimes stuck in an unresponsive state, hard reset it to resolve it

    SensorNode(&led, &temperature, &network).main_loop();

    return -1; // This will never be reached
}