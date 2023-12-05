#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <ranges>

#include <libopencm3/stm32/dma.h>
#include <libopencm3/stm32/usart.h>

#include "Logger.h"
#include "System.h"
#include "USART.h"
#include "interrupts.h"

// Give these secrets to the preprocessor with -D
#ifndef WIFI_AP
#define WIFI_AP "FAKE_AP"
#endif // !WIFI_AP

#ifndef WIFI_PASS
#define WIFI_PASS "FAKE_PASS"
#endif // !WIFI_PASS

#ifndef SERVER_IP
#define SERVER_IP "0.0.0.0"
#endif // !SERVER_IP

#ifndef SERVER_PORT
#define SERVER_PORT "666"
#endif // !SERVER_PORT

class Network {
public:
    virtual void reset() const = 0;
    virtual void connect_to_ap() const = 0;
    virtual void publish_measurement(double temperature) const = 0;
    virtual void test_connection() const = 0;
};

class ESP8266Network : public Network {
public:
    constexpr ESP8266Network(const Logger& logger, USARTWithDMA& usart) noexcept
        : logger(logger)
        , usart(usart)
    {
    }

    void reset() const override
    {
        send_command_dont_care("AT+RST");
        sensor_node_system::sleep_ms(2 * response_time); // Wait a bit so the ESP8266 has time to reset
        logger.info("Network reset!\n");
    }

    void connect_to_ap() const override
    {
        std::array<volatile char, 1024> buf {};
        if (send_command("AT+CWJAP_CUR=\"" WIFI_AP "\",\"" WIFI_PASS "\"", "OK", buf)) {
            logger.info("Network connection setup!\n");
        } else {
            logger.info("Failed to setup network connection!\n");
        }
    }

    void publish_measurement(double temperature) const override
    {
        std::array<volatile char, 1024> buf {};
        if (send_command("AT+CIPSTART=\"UDP\",\"" SERVER_IP "\"," SERVER_PORT, "OK", buf)) {
            // Send the temperature measurement
            char cipsend[20];
            snprintf(cipsend, 20, "AT+CIPSEND=%d", sizeof(temperature));
            send_command_dont_care(cipsend);
            send_data_dont_care({ reinterpret_cast<uint8_t*>(&temperature), sizeof(temperature) });
        }
    }

    void test_connection() const override { send_command_dont_care("AT"); }

private:
    const Logger& logger;
    USARTWithDMA& usart;

    constexpr static unsigned int response_time = 4000;

    void send_data_dont_care(std::span<uint8_t> data) const
    {
        for (auto& d : data) {
            usart.send_blocking(d);
        }
        usart.send_blocking("\r\n");
    }

    void send_command_dont_care(std::string_view cmd) const
    {
        usart.send_blocking(cmd);
        usart.send_blocking("\r\n");
    }

    [[nodiscard]] bool send_command(std::string_view cmd, std::string_view ok_response, std::span<volatile char> response_buf) const
    {
        using namespace std::literals; // std::string_view literals

        // Enable usart interrupts
        usart2_overrun_error = false;
        usart.error_interrupt(true);
        usart.rx_interrupt(true);
        // Enable usart DMA
        dma_buffer_full = false;
        dma_buffer_half = false;
        dma_error = false;
        usart.enable_rx_dma(reinterpret_cast<uint32_t>(response_buf.data()), response_buf.size());

        // Send the command
        send_command_dont_care(cmd);
        // Wait a bit for the response
        sensor_node_system::sleep_ms(response_time);

        // Disable DMA
        usart.disable_rx_dma();
        // Disable usart interrupts
        usart.error_interrupt(false);
        usart.rx_interrupt(false);

        // Check the response
        bool ok = false;
        // If we have an overrun error it means that we missed some output -> automatic fail
        // maybe you need a bigger buffer?
        if (!usart2_overrun_error) {
            // Check how many bytes were received
            auto count = usart.get_dma_count();
            auto read = response_buf.size() - count;

            // Split respone by line by line, check if any of the lines contains the OK response
            const std::string_view sv { const_cast<char*>(response_buf.data()), read };
            ok = std::ranges::any_of(sv | std::views::lazy_split("\r\n"sv),
                [ok_response](auto const& line) { return !std::ranges::search(line, ok_response).empty(); });
        }

        // Clear the overrun error if it happened
        usart2_overrun_error = false;

        return ok;
    }
};