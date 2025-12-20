#include "libs/SimpleMQTT/SimpleMQTT.h"
#include <array>
#include <doctest/doctest.h>
#include <string>

TEST_CASE("MQTT Connect Packet")
{
    std::array<std::byte, 128> packet;
    size_t len = SimpleMQTT::make_connect_packet(packet, "test_client", 10);

    // Expected length calculation:
    // Protocol Name "MQTT" (2 len + 4 str) = 6
    // Level (1) = 1
    // Flags (1) = 1
    // Keep Alive (2) = 2
    // Client ID "test_client" (2 len + 11 str) = 13
    // Total Remaining Length = 23

    // Fixed Header: 0x10 (CONNECT)
    // Remaining Length: 23 (0x17)

    CHECK(len == 2 + 23);
    CHECK(packet[0] == std::byte { 0x10 });
    CHECK(packet[1] == std::byte { 23 });

    // Protocol Name
    CHECK(packet[2] == std::byte { 0x00 });
    CHECK(packet[3] == std::byte { 0x04 });
    CHECK(packet[4] == std::byte { 'M' });
    CHECK(packet[5] == std::byte { 'Q' });
    CHECK(packet[6] == std::byte { 'T' });
    CHECK(packet[7] == std::byte { 'T' });

    // Level
    CHECK(packet[8] == std::byte { 0x04 });

    // Flags (Clean Session)
    CHECK(packet[9] == std::byte { 0x02 });

    // Keep Alive (10 = 0x000A)
    CHECK(packet[10] == std::byte { 0x00 });
    CHECK(packet[11] == std::byte { 0x0A });

    // Client ID
    CHECK(packet[12] == std::byte { 0x00 });
    CHECK(packet[13] == std::byte { 11 });
    std::string client_id_str(reinterpret_cast<const char*>(&packet[14]), 11);
    CHECK(client_id_str == "test_client");
}

TEST_CASE("MQTT Publish Packet QoS 0")
{
    std::array<std::byte, 128> packet;
    std::string message = "hello";
    std::span<const std::byte> payload(reinterpret_cast<const std::byte*>(message.data()), message.size());
    size_t len = SimpleMQTT::make_publish_packet(packet, "topic/test", payload);

    // Variable Header:
    // Topic "topic/test" (2 len + 10 str) = 12
    // No Packet ID for QoS 0

    // Payload: "hello" (5)

    // Total Remaining Length = 12 + 5 = 17

    CHECK(len == 2 + 17);
    CHECK(packet[0] == std::byte { 0x30 }); // PUBLISH, QoS 0
    CHECK(packet[1] == std::byte { 17 });

    // Topic
    CHECK(packet[2] == std::byte { 0x00 });
    CHECK(packet[3] == std::byte { 10 });
    std::string topic_str(reinterpret_cast<const char*>(&packet[4]), 10);
    CHECK(topic_str == "topic/test");

    // Payload
    std::string payload_str(reinterpret_cast<const char*>(&packet[14]), 5);
    CHECK(payload_str == "hello");
}

TEST_CASE("MQTT Publish Packet QoS 1")
{
    std::array<std::byte, 128> packet;
    std::string message = "m";
    std::span<const std::byte> payload(reinterpret_cast<const std::byte*>(message.data()), message.size());
    size_t len = SimpleMQTT::make_publish_packet(packet, "t", payload, true);

    // Variable Header:
    // Topic "t" (2 len + 1 str) = 3
    // Packet ID (2) = 2

    // Payload: "m" (1)

    // Total Remaining Length = 3 + 2 + 1 = 6

    CHECK(len == 2 + 6);
    CHECK(packet[0] == std::byte { 0x32 }); // PUBLISH, QoS 1 (Bit 1 set) -> 0x30 | 0x02 = 0x32
    CHECK(packet[1] == std::byte { 6 });

    // Topic
    CHECK(packet[2] == std::byte { 0x00 });
    CHECK(packet[3] == std::byte { 1 });
    CHECK(packet[4] == std::byte { 't' });

    // Packet ID (Dummy 1)
    CHECK(packet[5] == std::byte { 0x00 });
    CHECK(packet[6] == std::byte { 0x01 });

    // Payload
    CHECK(packet[7] == std::byte { 'm' });
}

TEST_CASE("MQTT Buffer Overflow protection check")
{
    std::array<std::byte, 10> packet; // Too small
    std::string message = "hello";
    std::span<const std::byte> payload(reinterpret_cast<const std::byte*>(message.data()), message.size());
    size_t len = SimpleMQTT::make_publish_packet(packet, "topic/test", payload);

    CHECK(len == 0);
}
