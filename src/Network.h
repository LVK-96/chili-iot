#pragma once

#include <algorithm>
#include <array>
#include <cstdio>
#include <ranges>
#include <string_view>

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

enum class SocketType { TCP, UDP };

class Network {
public:
    virtual void reset() const = 0;
    virtual void hard_reset() const = 0;
    virtual utils::ErrorCode init() = 0;
    [[nodiscard]] virtual bool get_ap_connected() = 0;
    [[nodiscard]] virtual utils::ErrorCode connect_to_ap() = 0;
    [[nodiscard]] virtual std::optional<unsigned int> connect_socket(
        SocketType sock_type, std::string_view addr, std::string_view port)
        = 0;
    [[nodiscard]] virtual utils::ErrorCode send_socket(unsigned int id, std::span<uint8_t> data) const = 0;
    [[nodiscard]] virtual utils::ErrorCode close_socket(unsigned int id) = 0;
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
        bluepill::async_wait_ms(5000);

        if constexpr (debug) {
            // Enable usart interrupts
            usart->error_interrupt(true);
            // Enable usart DMA
            usart->enable_rx_dma(reinterpret_cast<uintptr_t>(buf.data()), buf.size());

            reset_pin->port->set_pins(reset_pin->pin_nro);
            bluepill::async_wait_ms(reset_time); // Wait a bit so the ESP8266 has time to reset

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
            bluepill::async_wait_ms(reset_time); // Wait a bit so the ESP8266 has time to reset
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
            usart->enable_rx_dma(reinterpret_cast<uintptr_t>(buf.data()), buf.size());

            send_command_dont_care("AT+RST");
            bluepill::async_wait_ms(reset_time); // Wait a bit so the ESP8266 has time to reset

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
            bluepill::async_wait_ms(reset_time); // Wait a bit so the ESP8266 has time to reset
        }

        utils::logger.info("ESP8266 reset!\n");
    }

    utils::ErrorCode init() override
    {
        auto res = utils::ErrorCode::NETWORK_RESPONSE_NOT_OK_ERROR;
        reset_pin->port->set_pins(reset_pin->pin_nro);
        while (res != utils::ErrorCode::OK) {
            reset();
            res = test_msg();
        }
        return connect_to_ap();
    }

    [[nodiscard]] utils::ErrorCode connect_to_ap() override
    {
        utils::logger.info("Connecting to AP...\n");
        auto res = send_command("AT+CWJAP_CUR=\"" WIFI_AP "\",\"" WIFI_PASS "\"", "OK", buf, 10'000);
        if (res == utils::ErrorCode::OK) {
            utils::logger.info("Connencted to AP: " WIFI_AP "!\n");
            ap_connected = true;
        } else {
            utils::logger.error("Failed to connect to AP: " WIFI_AP "!\n");
        }

        return res;
    }

    [[nodiscard]] bool get_ap_connected() override { return ap_connected; }

    [[nodiscard]] utils::ErrorCode test_msg() const
    {
        utils::logger.info("Testing serial connection to ESP8266...\n");
        return send_command("AT", "OK", buf);
    }

    [[nodiscard]] std::optional<unsigned int> connect_socket(
        SocketType type, std::string_view addr, std::string_view port) override
    {
        std::optional<unsigned int> res = std::nullopt;
        if (ap_connected && (connections < max_connections)) {
            send_raw("AT+CIPSTART=");
            if (type == SocketType::UDP) {
                send_raw("\"UDP\"");
            } else { // TCP
                send_raw("\"TCP\"");
            }
            send_raw(",\"");
            send_raw(addr);
            send_raw("\",");
            send_raw(port);
            if (send_command("", "OK", buf, 5000) == utils::ErrorCode::OK) {
                connections++;
                unsigned int id = 0; // Only 1 connection at a time supported for now
                socket_connections[id] = true;
                res = id;
            }

            if ((send_command("AT+CIPMODE=1", "OK", buf, 5000) != utils::ErrorCode::OK)
                || (send_command("AT+CIPSEND", "OK", buf, 5000) != utils::ErrorCode::OK)) {
                utils::logger.error("Failed to set ESP8266 to transparent mode!\n");
                send_command_dont_care("AT+CIPCLOSE");
                res = std::nullopt;
            }
        }
        // Return socket id
        return res;
    }

    [[nodiscard]] utils::ErrorCode send_socket(unsigned int id, std::span<uint8_t> data) const override
    {
        if (ap_connected && socket_connected(id)) {
            send_raw(data);
            bluepill::async_wait_ms(50);
            return utils::ErrorCode::OK;
        }

        return utils::ErrorCode::NETWORK_RESPONSE_NOT_OK_ERROR;
    }

    [[nodiscard]] utils::ErrorCode close_socket(unsigned int id) override
    {
        if (ap_connected && socket_connected(id)) {
            send_command_dont_care("AT+CIPCLOSE");
            connections--;
            socket_connections[id] = false;
            return utils::ErrorCode::OK;
        }
        return utils::ErrorCode::NETWORK_RESPONSE_NOT_OK_ERROR;
    }

private:
    const USARTWithDMA* usart;
    const GPIOPin* reset_pin; // Reset when transitions from low -> high

    static constexpr bool debug = true;

    constexpr static unsigned int response_time = 10'000;
    constexpr static unsigned int reset_time = 10'000; // This thing is sometimes really slow to reset...
    constexpr static unsigned int break_time = 5;

    bool ap_connected = false;

    static constexpr unsigned int max_connections = 1;
    unsigned int connections = 0;
    std::array<bool, max_connections> socket_connections = { false };

    mutable std::array<volatile char, 2048> buf {};

    bool socket_connected(unsigned int id) const { return (id < max_connections) && socket_connections[id]; }

    void send_raw(std::span<uint8_t> bytes) const { usart->send_blocking(bytes); }

    void send_raw(uint8_t byte) const { usart->send_blocking(byte); }

    void send_raw(std::string_view msg) const { usart->send_blocking(msg); }

    void send_command_dont_care(std::string_view cmd) const
    {
        send_raw(cmd);
        send_raw("\r\n");
    }

    void send_data_dont_care(std::span<uint8_t> data) const
    {
        send_raw(data);
        send_raw("\r\n");
    }

    [[nodiscard]] utils::ErrorCode send_command(std::string_view cmd, std::string_view ok_response,
        std::span<volatile char> response_buf, unsigned int response_time_ms = 10'000) const
    {
        using namespace std::literals; // std::string_view literals
        // Enable usart interrupts
        usart->error_interrupt(true);
        // Enable usart DMA
        usart->enable_rx_dma(reinterpret_cast<uintptr_t>(response_buf.data()), response_buf.size());

        // Send the command
        send_command_dont_care(cmd);
        // Wait a bit for the response
        bluepill::async_wait_ms(response_time_ms);

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

class Socket {
public:
    Socket(Network* network)
        : network(network)
        , id(std::nullopt)
    {
    }

    [[nodiscard]] utils::ErrorCode connect(SocketType sock_type, std::string_view addr, std::string_view port)
    {
        // Connect this socket, network returns the assigned ID if succesfull
        id = network->connect_socket(sock_type, addr, port);

        auto res = utils::ErrorCode::NETWORK_RESPONSE_NOT_OK_ERROR;
        if (id) {
            res = utils::ErrorCode::OK;
        }
        return res;
    }

    [[nodiscard]] utils::ErrorCode close() const
    {
        auto res = utils::ErrorCode::NETWORK_RESPONSE_NOT_OK_ERROR;
        if (id) {
            res = network->close_socket(id.value());
        }
        return res;
    }

    [[nodiscard]] utils::ErrorCode send(std::span<uint8_t> data) const
    {
        auto res = utils::ErrorCode::NETWORK_RESPONSE_NOT_OK_ERROR;
        if (id) {
            res = network->send_socket(id.value(), data);
        }
        return res;
    }

private:
    Network* network;
    std::optional<int> id;
};
