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
#include "interrupts.h"
#include "utils.h"

enum class SocketType { TCP, UDP };

class INetwork {
public:
    virtual ~INetwork() = default;
    virtual utils::ErrorCode init() = 0;
    [[nodiscard]] virtual bool get_ap_connected() = 0;
    [[nodiscard]] virtual utils::ErrorCode connect_to_ap() = 0;
    virtual void disconnect_ap() const = 0;
    [[nodiscard]] virtual std::optional<unsigned int> connect_socket(
        SocketType sock_type, std::string_view addr, std::string_view port)
        = 0;
    [[nodiscard]] virtual utils::ErrorCode send_socket(unsigned int id, std::span<const std::byte> data) const = 0;
    [[nodiscard]] virtual utils::ErrorCode close_socket(unsigned int id) = 0;
};

class Socket {
public:
    Socket(INetwork* network)
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

    [[nodiscard]] utils::ErrorCode send(std::span<const std::byte> data) const
    {
        auto res = utils::ErrorCode::NETWORK_RESPONSE_NOT_OK_ERROR;
        if (id) {
            res = network->send_socket(id.value(), data);
        }
        return res;
    }

private:
    INetwork* network;
    std::optional<int> id;
};
