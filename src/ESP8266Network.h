#pragma once

#include <array>
#include <cstdio>
#include <functional>
#include <string_view>

#include "ATCommand.h"
#include "AtCommandProcessor.h"
#include "GPIO.h"
#include "Logger.h"
#include "System.h"
#include "interfaces/IDmaSerial.h"
#include "interfaces/INetwork.h"
#include "interrupts.h"
#include "utils.h"

// Give these secrets to the preprocessor with -D
#ifndef WIFI_AP
#define WIFI_AP "FAKE_AP"
#endif // !WIFI_AP

#ifndef WIFI_PASS
#define WIFI_PASS "FAKE_PASS"
#endif // !WIFI_PASS

class ESP8266Network final : public INetwork {
public:
    constexpr ESP8266Network(const IDmaSerial* usart, const GPIOPin* reset_pin, const IRTOS* rtos) noexcept
        : usart(usart)
        , reset_pin(reset_pin)
        , rtos(rtos)
        , at_processor(usart, rtos)
    {
    }

    utils::ErrorCode init() override
    {
        // Circular buffer RX DMA setup
        at_processor.start_rx_dma();
        // Echo AT commands off (default)
        std::function<utils::ErrorCode()> echo_on_or_off = [this] { return echo_off(); };

        // If we are debugging, enable command echo
        if constexpr (debug) {
            echo_on_or_off = [this] { return echo_on(); };
        }

        reset_pin->port->set_pins(reset_pin->pin_nro);
        rtos->delay(reset_time); // Wait a bit so the ESP8266 has time to boot

        // Enable the tx complete interrupt
        usart->clear_tx_transfer_complete_flag();
        // TODO: Cast to TaskHandle_t (void* -> void* or struct ptr, depending on FreeRTOS config)
        set_network_task_handle_for_usart2_interrupts(static_cast<TaskHandle_t>(rtos->get_current_task_handle()));
        usart->clear_sr_tc_bit(); // Clear the TC bit before we enable the interrupt
        usart->tx_complete_interrupt(true);

        while (test_msg() != utils::ErrorCode::OK || echo_on_or_off() != utils::ErrorCode::OK) {
            reset();
        }

        return connect_to_ap();
    }

    void disconnect_ap() const override
    {
        utils::logger.info("Disconnecting from AP...\n");
        at_processor.send_raw(esp8266::commands::DISCONNECT_AP);
    }

    [[nodiscard]] utils::ErrorCode connect_to_ap() override
    {
        utils::logger.info("Connecting to AP...\n");

        // Check if already connected
        auto res = at_processor.send_command(esp8266::commands::QUERY_AP_DEF, "+CWJAP_DEF:\"" WIFI_AP "\"");
        bool correct_ap_connected = res == utils::ErrorCode::OK;

        if (correct_ap_connected) {
            utils::logger.info("Already connected to AP: " WIFI_AP "!\n");
            ap_connected = true;
        }

        if (!correct_ap_connected) {
            disconnect_ap();
            res = at_processor.send_command(join_ap_def(WIFI_AP, WIFI_PASS), "OK");
            if (res == utils::ErrorCode::OK) {
                ap_connected = true;
            } else {
                utils::logger.error("Failed to connect to AP: " WIFI_AP "!\n");
            }
        }

        if (ap_connected)
            utils::logger.info("Connencted to AP: " WIFI_AP "!\n");

        return res;
    }

    [[nodiscard]] bool get_ap_connected() override { return ap_connected; }

    [[nodiscard]] utils::ErrorCode test_msg() const
    {
        utils::logger.info("Testing serial connection to ESP8266...\n");
        return at_processor.send_command(esp8266::commands::TEST, "OK");
    }

    [[nodiscard]] utils::ErrorCode echo_off() const
    {
        utils::logger.info("Turning off AT command echo...\n");
        return at_processor.send_command(esp8266::commands::ECHO_OFF, "OK");
    }

    [[nodiscard]] utils::ErrorCode echo_on() const
    {
        utils::logger.info("Turning on AT command echo...\n");
        return at_processor.send_command(esp8266::commands::ECHO_ON, "OK");
    }

    [[nodiscard]] std::optional<unsigned int> connect_socket(
        SocketType type, std::string_view addr, std::string_view port) override
    {
        std::optional<unsigned int> res = std::nullopt;
        if (ap_connected && (connections < max_connections)) {
            // Build command in a fixed-size rx_buffer to send in one DMA transaction
            constexpr size_t max_cmd_size
                = std::size(R"(AT+CIPSTART="UDP","255.255.255.255",65535)") + 2; // +2 for \r\n
            char cmd[max_cmd_size];
            const auto* type_str = (type == SocketType::UDP) ? "UDP" : "TCP";
            std::snprintf(cmd, max_cmd_size, "AT+CIPSTART=\"%s\",\"%.*s\",%.*s\r\n", type_str,
                static_cast<int>(addr.size()), addr.data(), static_cast<int>(port.size()), port.data());

            if (at_processor.send_command(esp8266::ATCommand(std::string_view(cmd)), "OK") == utils::ErrorCode::OK) {
                connections++;
                unsigned int id = 0; // Only 1 connection at a time supported for now
                socket_connections[id] = true;
                res = id;
            }

            if ((at_processor.send_command(esp8266::commands::TRANSPARENT_MODE, "OK") != utils::ErrorCode::OK)
                || (at_processor.send_command(esp8266::commands::START_SEND, "OK") != utils::ErrorCode::OK)) {
                utils::logger.error("Failed to set ESP8266 to transparent mode!\n");
                at_processor.send_raw(esp8266::commands::CLOSE_SOCKET);
                res = std::nullopt;
            }
        }
        // Return socket id
        return res;
    }

    [[nodiscard]] utils::ErrorCode send_socket(unsigned int id, std::span<const std::byte> data) const override
    {
        if (ap_connected && socket_connected(id)) {
            at_processor.send_raw(data, 100);
            return utils::ErrorCode::OK;
        }

        return utils::ErrorCode::NETWORK_RESPONSE_NOT_OK_ERROR;
    }

    [[nodiscard]] utils::ErrorCode close_socket(unsigned int id) override
    {
        if (ap_connected && socket_connected(id)) {
            at_processor.send_raw(esp8266::commands::CLOSE_SOCKET);
            connections--;
            socket_connections[id] = false;
            return utils::ErrorCode::OK;
        }
        return utils::ErrorCode::NETWORK_RESPONSE_NOT_OK_ERROR;
    }

private:
    const IDmaSerial* usart;
    const GPIOPin* reset_pin; // Reset when transitions from low -> high
    const IRTOS* rtos;

    static constexpr bool debug = true;

    constexpr static unsigned int response_time = 10'000;
    constexpr static unsigned int reset_time = 10'000; // This thing is sometimes really slow to reset...
    constexpr static unsigned int break_time = 5;

    bool ap_connected = false;

    static constexpr unsigned int max_connections = 1;
    unsigned int connections = 0;
    std::array<bool, max_connections> socket_connections = { false };

    AtCommandProcessor at_processor;

    bool socket_connected(unsigned int id) const { return (id < max_connections) && socket_connections[id]; }

    void hard_reset() const
    {
        utils::logger.info("Hard resetting ESP8266...\n");
        reset_pin->port->clear_pins(reset_pin->pin_nro);
        rtos->delay(100);
        reset_pin->port->set_pins(reset_pin->pin_nro);
        rtos->delay(reset_time);
    }

    void reset() const
    {
        utils::logger.info("Soft resetting ESP8266...\n");
        (void)at_processor.send_command(esp8266::commands::RESET, "ready", reset_time);
    }
};