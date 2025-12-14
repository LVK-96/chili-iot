#pragma once

#include <cstddef>
#include <span>
#include <string_view>

namespace esp8266 {

/// @brief Strong type wrapper for AT commands to ensure type safety
class ATCommand : public std::string_view {
public:
    using std::string_view::string_view; // Inherit constructors
    constexpr explicit ATCommand(std::string_view cmd)
        : std::string_view(cmd)
    {
    }

    constexpr operator std::span<const std::byte>() const
    {
        return std::span<const std::byte>(reinterpret_cast<const std::byte*>(data()), size());
    }
};

// Helper macro for compile-time string concatenation
#define AT_CMD(cmd) esp8266::ATCommand(std::string_view(cmd "\r\n"))

// Internal helper macros for parameterized commands
#define AT_JOIN_AP_IMPL(ssid, password)                                                                                \
    esp8266::ATCommand(std::string_view("AT+CWJAP=\"" ssid "\",\"" password "\"\r\n"))
#define AT_JOIN_AP_DEF_IMPL(ssid, password)                                                                            \
    esp8266::ATCommand(std::string_view("AT+CWJAP_DEF=\"" ssid "\",\"" password "\"\r\n"))
#define AT_START_CONNECTION_IMPL(type, addr, port)                                                                     \
    esp8266::ATCommand(std::string_view("AT+CIPSTART=\"" type "\",\"" addr "\"," port "\r\n"))
#define AT_SEND_LENGTH_IMPL(length) esp8266::ATCommand(std::string_view("AT+CIPSEND=" length "\r\n"))

// AT commands namespace - contains both simple and parameterized commands
namespace commands {
    // ========== Simple Commands (no parameters) ==========
    constexpr ATCommand RESET = AT_CMD("AT+RST");
    constexpr ATCommand DISCONNECT_AP = AT_CMD("AT+CWQAP");
    constexpr ATCommand CLOSE_SOCKET = AT_CMD("AT+CIPCLOSE");
    constexpr ATCommand TEST = AT_CMD("AT");
    constexpr ATCommand ECHO_OFF = AT_CMD("ATE0");
    constexpr ATCommand ECHO_ON = AT_CMD("ATE1");
    constexpr ATCommand TRANSPARENT_MODE = AT_CMD("AT+CIPMODE=1");
    constexpr ATCommand START_SEND = AT_CMD("AT+CIPSEND");
    constexpr ATCommand QUERY_AP_DEF = AT_CMD("AT+CWJAP_DEF?");

// ========== Parameterized Command Builders (flexible) ==========
/// @brief Build AT+CWJAP command at compile time
/// @param ssid WiFi SSID (must be a macro or string literal)
/// @param password WiFi password (must be a macro or string literal)
#define join_ap(ssid, password) AT_JOIN_AP_IMPL(ssid, password)

/// @brief Build AT+CWJAP_DEF command at compile time
/// @param ssid WiFi SSID (must be a macro or string literal)
/// @param password WiFi password (must be a macro or string literal)
#define join_ap_def(ssid, password) AT_JOIN_AP_DEF_IMPL(ssid, password)

/// @brief Build AT+CIPSTART command at compile time
/// @param type Connection type: "TCP" or "UDP"
/// @param addr IP address or hostname
/// @param port Port number (as string)
#define start_connection(type, addr, port) AT_START_CONNECTION_IMPL(type, addr, port)

/// @brief Build AT+CIPSEND command at compile time
/// @param length Number of bytes (as string)
#define send_length(length) AT_SEND_LENGTH_IMPL(length)

// ========== Hardcoded Commands (convenience, if config macros defined) ==========
// If WIFI_SSID and WIFI_PASSWORD are defined, create a hardcoded JOIN_AP command
#if defined(WIFI_SSID) && defined(WIFI_PASSWORD)
    constexpr ATCommand JOIN_AP = join_ap(WIFI_SSID, WIFI_PASSWORD);
#endif

// If SERVER_IP and SERVER_PORT are defined, create hardcoded connection commands
#if defined(SERVER_IP) && defined(SERVER_PORT)
    constexpr ATCommand START_TCP_CONNECTION = start_connection("TCP", SERVER_IP, SERVER_PORT);
    constexpr ATCommand START_UDP_CONNECTION = start_connection("UDP", SERVER_IP, SERVER_PORT);
#endif
} // namespace commands

} // namespace esp8266
