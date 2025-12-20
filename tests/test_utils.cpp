#include "utils.h"
#include <doctest/doctest.h>

TEST_CASE("Utility mask functions")
{
    SUBCASE("mask32 generates correct masks")
    {
        CHECK(utils::mask32<0>() == 0x00000000);
        CHECK(utils::mask32<1>() == 0x00000001);
        CHECK(utils::mask32<8>() == 0x000000FF);
        CHECK(utils::mask32<16>() == 0x0000FFFF);
        CHECK(utils::mask32<24>() == 0x00FFFFFF);
        CHECK(utils::mask32<32>() == 0xFFFFFFFF);
    }

    SUBCASE("mask16 generates correct masks")
    {
        CHECK(utils::mask16<0>() == 0x0000);
        CHECK(utils::mask16<1>() == 0x0001);
        CHECK(utils::mask16<8>() == 0x00FF);
        CHECK(utils::mask16<12>() == 0x0FFF);
        CHECK(utils::mask16<16>() == 0xFFFF);
    }

    SUBCASE("mask8 generates correct masks")
    {
        CHECK(utils::mask8<0>() == 0x00);
        CHECK(utils::mask8<1>() == 0x01);
        CHECK(utils::mask8<4>() == 0x0F);
        CHECK(utils::mask8<7>() == 0x7F);
        CHECK(utils::mask8<8>() == 0xFF);
    }

    SUBCASE("Masks are constexpr")
    {
        // This test verifies that masks can be used in constexpr contexts
        constexpr uint32_t mask = utils::mask32<16>();
        CHECK(mask == 0x0000FFFF);

        constexpr uint16_t mask16 = utils::mask16<8>();
        CHECK(mask16 == 0x00FF);

        constexpr uint8_t mask8 = utils::mask8<4>();
        CHECK(mask8 == 0x0F);
    }
}

TEST_CASE("ErrorCode enum values")
{
    CHECK(static_cast<uint8_t>(utils::ErrorCode::OK) == 0);
    CHECK(static_cast<uint8_t>(utils::ErrorCode::TEMPERATURE_INIT_ERROR) == 1);
    CHECK(static_cast<uint8_t>(utils::ErrorCode::USART_NOT_SETUP_ERROR) == 10);
    CHECK(static_cast<uint8_t>(utils::ErrorCode::NETWORK_RESPONSE_NOT_OK_ERROR) == 20);
    CHECK(static_cast<uint8_t>(utils::ErrorCode::NETWORK_RESPONSE_OVERRUN_ERROR) == 21);
    CHECK(static_cast<uint8_t>(utils::ErrorCode::UNEXPECTED_ERROR) == 255);
}
