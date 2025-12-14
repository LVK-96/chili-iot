#pragma once

#include <algorithm>
#include <array>
#include <bit>
#include <cstdint>
#include <cstring>
#include <span>
#include <string_view>

namespace SimpleMQTT {

enum class PacketType : uint8_t {
    CONNECT = 0x10,
    CONNACK = 0x20,
    PUBLISH = 0x30,
    PUBACK = 0x40,
    PUBREC = 0x50,
    PUBREL = 0x60,
    PUBCOMP = 0x70,
    SUBSCRIBE = 0x80,
    SUBACK = 0x90,
    UNSUBSCRIBE = 0xA0,
    UNSUBACK = 0xB0,
    PINGREQ = 0xC0,
    PINGRESP = 0xD0,
    DISCONNECT = 0xE0
};

/*  MQTT Packet Structure:
 *
 *      Bit   7   6   5   4   3   2   1   0
 *          +---+---+---+---+---+---+---+---+
 *  Byte 1  |   Packet Type |     Flags     |
 *          +---+---+---+---+---+---+---+---+
 *  Byte 2  | C |    Remaining Length       |  (C = Continuation Bit)
 *          +---+---+---+---+---+---+---+---+
 *  Byte 3  | C |    Remaining Length       |  (Optional)
 *          +---+---+---+---+---+---+---+---+
 *  ...     | C |    Remaining Length       |  (Up to 4 bytes total)
 *          +---+---+---+---+---+---+---+---+
 *          |                               |
 *          |        Variable Header        |
 *          |                               |
 *          +-------------------------------+
 *          |                               |
 *          |            Payload            |
 *          |                               |
 *          +-------------------------------+
 */

namespace {
    void write_byte(std::span<std::byte> buffer, unsigned int& offset, const std::byte byte)
    {
        buffer[offset++] = byte;
    }

    void write_bytes(std::span<std::byte> buffer, unsigned int& offset, std::span<const std::byte> data)
    {
        std::ranges::copy(data, buffer.begin() + offset);
        offset += data.size();
    }

    void write_u16_be(std::span<std::byte> buffer, unsigned int& offset, uint16_t value)
    {
        if constexpr (std::endian::native == std::endian::little) {
            value = std::byteswap(value);
        }
        write_bytes(buffer, offset, std::span<const std::byte>(reinterpret_cast<const std::byte*>(&value), 2));
    }

    void write_string(std::span<std::byte> buffer, unsigned int& offset, std::string_view str)
    {
        write_u16_be(buffer, offset, static_cast<uint16_t>(str.size()));
        write_bytes(
            buffer, offset, std::span<const std::byte>(reinterpret_cast<const std::byte*>(str.data()), str.size()));
    }

    struct RemainingLengthEncoding {
        std::array<std::byte, 4> bytes;
        unsigned int size;
    };

    constexpr RemainingLengthEncoding get_remaining_length_encoding(size_t length)
    {
        // MQTT remaining length encoding:
        // - Encodes the total length of Variable Header + Payload.
        // - Uses 1 to 4 bytes.
        // - Each byte uses the lower 7 bits (0-6) for data.
        // - Bit 7 (C) is the continuation flag:
        //   - 1: More length bytes follow.
        //   - 0: This is the last length byte.
        RemainingLengthEncoding result = {};

        result.size = 0;
        do {
            uint8_t encoded_byte = length & 0x7F;
            length >>= 7;
            if (length > 0) {
                encoded_byte |= 0x80;
            }
            result.bytes[result.size++] = static_cast<std::byte>(encoded_byte);
        } while (length > 0 && result.size < 4);
        return result;
    }
} // Anonymous namespace

inline unsigned int make_connect_packet(
    std::span<std::byte> buffer, std::string_view client_id, uint16_t keep_alive = 60)
{
    /* Connect Variable Header:
     *      Bit   7   6   5   4   3   2   1   0
     *          +---+---+---+---+---+---+---+---+
     *  Byte 1  | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | Length MSB (0)
     *          +---+---+---+---+---+---+---+---+
     *  Byte 2  | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 0 | Length LSB (4)
     *          +---+---+---+---+---+---+---+---+
     *  Byte 3  | 0 | 1 | 0 | 0 | 1 | 1 | 0 | 1 | "M"
     *          +---+---+---+---+---+---+---+---+
     *  Byte 4  | 0 | 1 | 0 | 1 | 0 | 0 | 0 | 1 | "Q"
     *          +---+---+---+---+---+---+---+---+
     *  Byte 5  | 0 | 1 | 0 | 1 | 0 | 1 | 0 | 0 | "T"
     *          +---+---+---+---+---+---+---+---+
     *  Byte 6  | 0 | 1 | 0 | 1 | 0 | 1 | 0 | 0 | "T"
     *          +---+---+---+---+---+---+---+---+
     *  Byte 7  | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 0 | Protocol Level (4)
     *          +---+---+---+---+---+---+---+---+
     *  Byte 8  | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | Flags: Clean Session
     *          +---+---+---+---+---+---+---+---+
     *  Byte 9  |      Keep Alive MSB           |
     *          +---+---+---+---+---+---+---+---+
     *  Byte 10 |      Keep Alive LSB           |
     *          +---+---+---+---+---+---+---+---+
     *  Payload:
     *          +---+---+---+---+---+---+---+---+
     *  Byte 11 |      Client ID Length MSB     |
     *          +---+---+---+---+---+---+---+---+
     *  Byte 12 |      Client ID Length LSB     |
     *          +---+---+---+---+---+---+---+---+
     *  ...     |         Client ID             |
     *  ...     |          ...                  |
     *          +---+---+---+---+---+---+---+---+
     */

    // Calculate variable header and payload size first to determine Remaining Length
    // Variable Header: Protocol Name (6) + Level (1) + Flags (1) + Keep Alive (2) = 10 bytes
    // Payload: Client ID (2 + len)
    constexpr unsigned int variable_header_size = 10;
    const unsigned int payload_size = 2 + client_id.size();
    const unsigned int remaining_length = variable_header_size + payload_size;
    const auto length_encoding = get_remaining_length_encoding(remaining_length);
    const unsigned int total_size = 1 + length_encoding.size + remaining_length;

    if (total_size > buffer.size()) {
        return 0; // Buffer too small
    }

    unsigned int offset = 0;
    // Fixed Header
    write_byte(buffer, offset, static_cast<std::byte>(PacketType::CONNECT));
    write_bytes(buffer, offset, std::span<const std::byte>(length_encoding.bytes.data(), length_encoding.size));

    // Variable Header
    // Protocol Name
    write_string(buffer, offset, "MQTT");

    // Protocol Level (4 for 3.1.1) and Connect Flags (Clean Session: 0x02)
    const std::array<std::byte, 2> level_and_flags = { std::byte { 4 }, std::byte { 0x02 } };
    write_bytes(buffer, offset, level_and_flags);

    // Keep Alive
    write_u16_be(buffer, offset, keep_alive);

    // Payload: Client ID
    write_string(buffer, offset, client_id);

    return offset;
}

inline unsigned int make_publish_packet(
    std::span<std::byte> buffer, std::string_view topic, std::span<const std::byte> payload, bool qos1 = false)
{
    /* Publish Variable Header:
     *      Bit   7   6   5   4   3   2   1   0
     *          +---+---+---+---+---+---+---+---+
     *  Byte 1  |      Topic Length MSB         |
     *          +---+---+---+---+---+---+---+---+
     *  Byte 2  |      Topic Length LSB         |
     *          +---+---+---+---+---+---+---+---+
     *  ...     |         Topic Name            |
     *  ...     |          ...                  |
     *          +---+---+---+---+---+---+---+---+
     * If QoS > 0:
     *          +---+---+---+---+---+---+---+---+
     *  Byte N  |      Packet ID MSB            |
     *          +---+---+---+---+---+---+---+---+
     *  Byte N+1|      Packet ID LSB            |
     *          +---+---+---+---+---+---+---+---+
     *  Payload:
     *          +---+---+---+---+---+---+---+---+
     *  ...     |         Message               |
     *  ...     |          ...                  |
     *          +---+---+---+---+---+---+---+---+
     */

    // Calculate Remaining Length
    // Variable Header: Topic Name (2 + len) + Packet ID (2 if QoS > 0)
    unsigned int variable_header_size = 2 + topic.size();
    if (qos1) {
        variable_header_size += 2;
    }
    const unsigned int payload_size = payload.size();
    const unsigned int remaining_length = variable_header_size + payload_size;

    // Determine bytes needed for Remaining Length encoding (1-4 bytes)
    const auto length_encoding = get_remaining_length_encoding(remaining_length);

    // Does our data fit in the allocated buffer?
    const unsigned int total_size = 1 + length_encoding.size + remaining_length;
    if (total_size > buffer.size()) {
        return 0; // Buffer too small
    }

    unsigned int offset = 0;

    // Fixed Header
    auto first_byte = static_cast<std::byte>(PacketType::PUBLISH);
    if (qos1) {
        first_byte |= std::byte { 0x02 }; // QoS 1
    }
    write_byte(buffer, offset, first_byte);
    write_bytes(buffer, offset, std::span<const std::byte>(length_encoding.bytes.data(), length_encoding.size));

    // Variable Header
    // Topic Name
    write_string(buffer, offset, topic);

    // Packet Identifier (only if QoS > 0)
    if (qos1) {
        // Just use a dummy ID for now
        write_u16_be(buffer, offset, 1);
    }

    // Payload
    write_bytes(buffer, offset, payload);

    return offset;
}

} // namespace SimpleMQTT
