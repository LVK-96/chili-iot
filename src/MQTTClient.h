#pragma once

#include <array>
#include <span>
#include <string_view>

#include "Network.h"
#include "libs/SimpleMQTT/SimpleMQTT.h"

class MQTTClient {
public:
    explicit MQTTClient(Network& network)
        : socket(&network)
    {
    }

    utils::ErrorCode connect(std::string_view client_id, std::string_view host, std::string_view port)
    {
        // TCP connect
        if (socket.connect(SocketType::TCP, host, port) != utils::ErrorCode::OK) {
            return utils::ErrorCode::NETWORK_RESPONSE_NOT_OK_ERROR;
        }

        // MQTT CONNECT calls
        std::array<std::byte, 256> buffer = {};
        const auto len = SimpleMQTT::make_connect_packet(buffer, client_id);
        if (len == 0) {
            return utils::ErrorCode::MEMORY_ERROR; // Buffer too small
        }

        return send_packet(std::span<std::byte>(buffer.data(), len));
    }

    utils::ErrorCode publish(std::string_view topic, std::span<const std::byte> payload)
    {
        std::array<std::byte, 256> buffer = {};
        const auto len = SimpleMQTT::make_publish_packet(buffer, topic, payload);
        if (len == 0) {
            return utils::ErrorCode::MEMORY_ERROR; // Buffer too small
        }
        return send_packet(std::span<std::byte>(buffer.data(), len));
    }

private:
    Socket socket;

    utils::ErrorCode send_packet(std::span<const std::byte> packet) { return socket.send(packet); }
};
