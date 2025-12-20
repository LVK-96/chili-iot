#pragma once

#include <cstddef>
#include <optional>
#include <span>
#include <string_view>

#include "Logger.h"
#include "interfaces/INetwork.h"
#include "utils.h"

/// Mock network that always succeeds - for QEMU simulation/testing RTOS task interactions
class MockNetwork final : public INetwork {
public:
    utils::ErrorCode init() override
    {
        utils::logger.info("MockNetwork: init() -> OK\n");
        ap_connected = true;
        return utils::ErrorCode::OK;
    }

    [[nodiscard]] bool get_ap_connected() override { return ap_connected; }

    [[nodiscard]] utils::ErrorCode connect_to_ap() override
    {
        utils::logger.info("MockNetwork: connect_to_ap() -> OK\n");
        ap_connected = true;
        return utils::ErrorCode::OK;
    }

    void disconnect_ap() const override { utils::logger.info("MockNetwork: disconnect_ap()\n"); }

    [[nodiscard]] std::optional<unsigned int> connect_socket(
        SocketType sock_type, std::string_view addr, std::string_view port) override
    {
        (void)sock_type;
        utils::logger.info("MockNetwork: connect_socket(%.*s:%.*s) -> socket 0\n", static_cast<int>(addr.size()),
            addr.data(), static_cast<int>(port.size()), port.data());
        socket_connected = true;
        return 0; // Always return socket ID 0
    }

    [[nodiscard]] utils::ErrorCode send_socket(unsigned int id, std::span<const std::byte> data) const override
    {
        (void)id;
        utils::logger.info("MockNetwork: send_socket(%u bytes) -> OK\n", static_cast<unsigned>(data.size()));
        return utils::ErrorCode::OK;
    }

    [[nodiscard]] utils::ErrorCode close_socket(unsigned int id) override
    {
        (void)id;
        utils::logger.info("MockNetwork: close_socket() -> OK\n");
        socket_connected = false;
        return utils::ErrorCode::OK;
    }

private:
    bool ap_connected = false;
    bool socket_connected = false;
};
