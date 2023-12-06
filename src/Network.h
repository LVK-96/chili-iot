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
    virtual sensor_node_system::ErrorCode connect_to_ap() const = 0;
    virtual sensor_node_system::ErrorCode publish_measurement(double temperature) const = 0;
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
        logger.info("Resetting ESP8266...\n");
        send_command_dont_care("AT+RST");
        sensor_node_system::sleep_ms(reset_time); // Wait a bit so the ESP8266 has time to reset
        logger.info("ESP8266 reset!\n");
    }

    sensor_node_system::ErrorCode connect_to_ap() const override
    {
        sensor_node_system::ErrorCode res = sensor_node_system::ErrorCode::OK;
        std::array<volatile char, 128> buf {};
        if ((res = send_command("AT+CWJAP_CUR=\"" WIFI_AP "\",\"" WIFI_PASS "\"", "OK", buf))
            == sensor_node_system::ErrorCode::OK) {
            logger.info("Connencted to AP: " WIFI_AP "!\n");
        } else {
            logger.error("Failed to connect to AP: " WIFI_AP "!\n");
        }

        return res;
    }

    sensor_node_system::ErrorCode publish_measurement(double temperature) const override
    {
        sensor_node_system::ErrorCode res = sensor_node_system::ErrorCode::OK;
        std::array<volatile char, 64> buf {};
        // Open the UDP connetion
        if ((res = send_command("AT+CIPSTART=\"UDP\",\"" SERVER_IP "\"," SERVER_PORT, "OK", buf))
            == sensor_node_system::ErrorCode::OK) {
            char cipsend[20];
            // The size of the data to send
            snprintf(cipsend, 20, "AT+CIPSEND=%d", sizeof(temperature));
            send_command_dont_care(cipsend);
            sensor_node_system::sleep_ms(break_time);
            // Send the temperature measurement
            send_data_dont_care({ reinterpret_cast<uint8_t*>(&temperature), sizeof(temperature) });
            // Close the UDP connection
            sensor_node_system::sleep_ms(break_time);
            send_command_dont_care("AT+CIPCLOSE");
            logger.info("Sent temperature measurement\n");
        } else {
            logger.error("Failed to send the temperature measurement\n");
        }

        return res;
    }

    void test_connection() const override { send_command_dont_care("AT"); }

private:
    const Logger& logger;
    USARTWithDMA& usart;

    constexpr static unsigned int response_time = 6000;
    constexpr static unsigned int reset_time = 8000;
    constexpr static unsigned int break_time = 100;

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

    [[nodiscard]] sensor_node_system::ErrorCode send_command(
        std::string_view cmd, std::string_view ok_response, std::span<volatile char> response_buf) const
    {
        using namespace std::literals; // std::string_view literals

        // Enable usart interrupts
        usart.error_interrupt(true);
        // Enable usart DMA
        usart.enable_rx_dma(reinterpret_cast<uint32_t>(response_buf.data()), response_buf.size());

        // Send the command
        send_command_dont_care(cmd);
        // Wait a bit for the response
        sensor_node_system::sleep_ms(response_time);

        // Disable DMA
        usart.disable_rx_dma();
        // Disable usart interrupts
        usart.error_interrupt(false);

        // Check the response
        auto res = sensor_node_system::ErrorCode::OK;
        // If we have an overrun error it means that we missed some output -> automatic fail
        // maybe you need a bigger buffer?
        unsigned int read = 0;
        if (!usart2_overrun_error) {
            // Check how many bytes were received
            auto count = usart.get_dma_count();
            read = response_buf.size() - count;

            // Split respone by line by line, check if any of the lines contains the OK response
            const std::string_view sv { const_cast<char*>(response_buf.data()), read };
            if (!(std::ranges::any_of(sv | std::views::lazy_split("\r\n"sv),
                    [ok_response](auto const& line) { return !std::ranges::search(line, ok_response).empty(); }))) {
                res = sensor_node_system::ErrorCode::NETWORK_RESPONSE_NOT_OK_ERROR;
            }
        }

        if (usart2_overrun_error) {
            logger.error("ESP8266 response caused an USART overrun error! Is the reponse buffer big enough?\n");
            res = sensor_node_system::ErrorCode::NETWORK_RESPONSE_OVERRUN_ERROR;
        }

        return res;
    }
};