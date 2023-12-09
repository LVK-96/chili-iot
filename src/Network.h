#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <ranges>
#include <string>

#include <libopencm3/stm32/dma.h>
#include <libopencm3/stm32/usart.h>

#include "GPIO.h"
#include "Logger.h"
#include "System.h"
#include "USART.h"
#include "interrupts.h"
#include "utils.h"

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
    virtual void hard_reset() const = 0;
    [[nodiscard]] virtual utils::ErrorCode connect_to_ap() const = 0;
    [[nodiscard]] virtual utils::ErrorCode connect_to_server() const = 0;
    virtual void disconnect_from_server() const = 0;
    //[[nodiscard]] virtual utils::ErrorCode publish_measurement(double temperature) const = 0;
    virtual void publish_measurement(double temperature) const = 0;
    virtual void test_connection() const = 0;
};

class ESP8266Network final : public Network {
public:
    constexpr ESP8266Network(const USARTWithDMA* usart, const GPIOPin* reset_pin) noexcept
        : usart(usart)
        , reset_pin(reset_pin)
    {
    }

    void hard_reset() const override
    {
        utils::logger.info("Hard resetting ESP8266...\n");

        reset_pin->port->clear_pins(reset_pin->pin_nro);
        bluepill::busy_wait_ms(1000);

        if constexpr (debug) {
            // Enable usart interrupts
            usart->error_interrupt(true);
            // Enable usart DMA
            usart->enable_rx_dma(reinterpret_cast<uint32_t>(buf.data()), buf.size());

            reset_pin->port->set_pins(reset_pin->pin_nro);
            bluepill::busy_wait_ms(reset_time); // Wait a bit so the ESP8266 has time to reset

            // Disable DMA
            usart->disable_rx_dma();
            // Disable usart interrupts
            usart->error_interrupt(false);
            auto count = usart->get_dma_count();
            unsigned read = buf.size() - count; // Check how many bytes were received
            const std::string_view sv { const_cast<char*>(buf.data()), read };
            utils::logger.info("Printing the received data (if any)...\n");
            if (read > 0) {
                utils::logger.log(sv);
            }
            utils::logger.log("\n");
        } else {
            reset_pin->port->set_pins(reset_pin->pin_nro);
            bluepill::busy_wait_ms(reset_time); // Wait a bit so the ESP8266 has time to reset
        }

        utils::logger.info("ESP8266 reset!\n");
    }

    void reset() const override
    {
        utils::logger.info("Soft resetting ESP8266...\n");

        if constexpr (debug) {
            // Enable usart interrupts
            usart->error_interrupt(true);
            // Enable usart DMA
            usart->enable_rx_dma(reinterpret_cast<uint32_t>(buf.data()), buf.size());

            send_command_dont_care("AT+RST");
            bluepill::busy_wait_ms(reset_time); // Wait a bit so the ESP8266 has time to reset

            // Disable DMA
            usart->disable_rx_dma();
            // Disable usart interrupts
            usart->error_interrupt(false);
            auto count = usart->get_dma_count();
            unsigned read = buf.size() - count; // Check how many bytes were received
            const std::string_view sv { const_cast<char*>(buf.data()), read };
            utils::logger.info("Printing the received data (if any)...\n");
            if (read > 0) {
                utils::logger.log(sv);
            }
            utils::logger.log("\n");
        } else {
            send_command_dont_care("AT+RST");
            bluepill::busy_wait_ms(reset_time); // Wait a bit so the ESP8266 has time to reset
        }

        utils::logger.info("ESP8266 reset!\n");
    }

    [[nodiscard]] utils::ErrorCode connect_to_ap() const override
    {
        utils::logger.info("Connecting to AP...\n");
        auto res = send_command("AT+CWJAP_CUR=\"" WIFI_AP "\",\"" WIFI_PASS "\"", "OK", buf);
        if (res == utils::ErrorCode::OK) {
            utils::logger.info("Connencted to AP: " WIFI_AP "!\n");
        } else {
            utils::logger.error("Failed to connect to AP: " WIFI_AP "!\n");
        }

        return res;
    }

    [[nodiscard]] utils::ErrorCode connect_to_server() const override
    {
        // Open the UDP connetion
        auto res = send_command("AT+CIPSTART=\"UDP\",\"" SERVER_IP "\"," SERVER_PORT, "OK", buf);
        if (res == utils::ErrorCode::OK) {
            send_command_dont_care("AT+CIPMODE=1");
            bluepill::busy_wait_ms(break_time);
            send_command_dont_care("AT+CIPSEND");
        }
        return res;
    }

    void disconnect_from_server() const override
    {
        send_command_dont_care("+++");
        bluepill::busy_wait_ms(break_time);
        send_command_dont_care("AT+CIPCLOSE");
    }

    void publish_measurement(double temperature) const override
    {
        send_data_dont_care({ reinterpret_cast<uint8_t*>(&temperature), sizeof(temperature) });
        utils::logger.info("Sent temperature measurement\n");
    }

    void test_connection() const override { send_command_dont_care("AT"); }

private:
    const USARTWithDMA* usart;
    const GPIOPin* reset_pin; // Reset when transitions from low -> high

    static constexpr bool debug = true;

    constexpr static unsigned int response_time = 6000;
    constexpr static unsigned int reset_time = 10'000; // This thing is sometimes really slow to reset...
    constexpr static unsigned int break_time = 5;

    mutable std::array<volatile char, 512> buf {};

    void send_data_dont_care(std::span<uint8_t> data) const
    {
        for (auto& d : data) {
            usart->send_blocking(d);
        }
        usart->send_blocking("\r\n");
    }

    void send_command_dont_care(std::string_view cmd) const
    {
        usart->send_blocking(cmd);
        usart->send_blocking("\r\n");
    }

    [[nodiscard]] utils::ErrorCode send_command(
        std::string_view cmd, std::string_view ok_response, std::span<volatile char> response_buf) const
    {
        using namespace std::literals; // std::string_view literals

        // Enable usart interrupts
        usart->error_interrupt(true);
        // Enable usart DMA
        usart->enable_rx_dma(reinterpret_cast<uint32_t>(response_buf.data()), response_buf.size());

        // Send the command
        send_command_dont_care(cmd);
        // Wait a bit for the response
        bluepill::busy_wait_ms(response_time);

        // Disable DMA
        usart->disable_rx_dma();
        // Disable usart interrupts
        usart->error_interrupt(false);

        // Check the response
        auto res = utils::ErrorCode::OK;
        auto count = usart->get_dma_count();
        unsigned read = response_buf.size() - count; // Check how many bytes were received
        const std::string_view sv { const_cast<char*>(response_buf.data()), read };
        if (!usart2_overrun_error) {
            // Split respone by line by line, check if any of the lines contains the OK response
            if (!(std::ranges::any_of(sv | std::views::lazy_split("\r\n"sv),
                    [ok_response](auto const& line) { return !std::ranges::search(line, ok_response).empty(); }))) {
                res = utils::ErrorCode::NETWORK_RESPONSE_NOT_OK_ERROR;
            }
        } else { // Overrun error happened, something went wrong
            utils::logger.info("ESP8266 response caused an USART overrun error! Is the reponse buffer big enough?\n");
            res = utils::ErrorCode::NETWORK_RESPONSE_OVERRUN_ERROR;
        }

        if constexpr (debug) {
            bool error = res != utils::ErrorCode::OK;
            if (error) {
                utils::logger.error("Printing the received data (if any)...\n");
            } else {
                utils::logger.info("Printing the received data (if any)...\n");
            }
            if (read > 0) {
                utils::logger.log(sv);
            }
            utils::logger.log("\n");
        }

        return res;
    }
};