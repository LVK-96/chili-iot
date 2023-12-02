#pragma once

#include <array>
#include <cstdio>

#include <libopencm3/stm32/dma.h>

#include "Logger.h"
#include "System.h"
#include "USART.h"

class Network {
public:
    Network(const Logger& logger, const USART& usart)
        : logger(logger)
        , usart(usart)
    {
    }
    void setup() const { }
    void test_connection() const
    {
        printf("Sending ATE0\n");
        usart.send_blocking("ATE0\r\n");
        sensor_node_system::sleep_ms(100);
        usart.send_blocking("AT\r\n");
    }

private:
    const Logger& logger;
    const USART& usart;
};