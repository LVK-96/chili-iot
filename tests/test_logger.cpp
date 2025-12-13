#include "Logger.h"
#include "USART.h"
#include "doctest.h"
#include "utils.h" // New include

#include <cstdint> // New include
#include <string>

#include "test_events.h"
#include <algorithm>
#include <vector>

TEST_CASE("USARTLogger sends expected bytes")
{
    test_event_clear();

    static volatile std::atomic_bool overrun_error_flag(false);
    static volatile std::atomic_bool tx_transfer_complete_flag(false);
    USART usart(BluePillUSART::_2, RCC_USART2, RST_USART2, &overrun_error_flag, &tx_transfer_complete_flag);
    USARTLogger logger(Logger::LogLevel::INFO, &usart);

    logger.info("hi");

    // Expect "INFO: hi" -> I N F O : <space> h i
    const std::string expected = "INFO: hi";

    auto events = test_event_get_all();
    auto tx_it = std::find_if(
        events.begin(), events.end(), [](const TestEvent& e) { return e.type == TestEventType::UsartTx; });
    REQUIRE(tx_it != events.end());

    REQUIRE(tx_it->data.size() == expected.size());
    for (size_t i = 0; i < expected.size(); ++i) {
        CHECK(tx_it->data[i] == static_cast<uint8_t>(expected[i]));
    }
}

TEST_CASE("Logger sends formatted messages via USART")
{
    test_event_clear();

    static volatile std::atomic_bool overrun_error_flag(false);
    static volatile std::atomic_bool tx_transfer_complete_flag(false);

    // Create a logger instance using USART2
    USART usart2(BluePillUSART::_2, RCC_USART2, RST_USART2, &overrun_error_flag, &tx_transfer_complete_flag);
    USARTLogger logger(Logger::LogLevel::INFO, &usart2);

    logger.log(Logger::LogLevel::INFO, "Hello %s", "World");

    // Expected: INFO: Hello World
    std::string expected_msg = "INFO: Hello World";
    std::vector<uint8_t> expected(expected_msg.begin(), expected_msg.end());

    auto events = test_event_get_all();
    auto tx_it = std::find_if(
        events.begin(), events.end(), [](const TestEvent& e) { return e.type == TestEventType::UsartTx; });
    REQUIRE(tx_it != events.end());

    REQUIRE(tx_it->data.size() == expected.size());
    for (size_t i = 0; i < expected.size(); ++i) {
        CHECK(tx_it->data[i] == static_cast<uint8_t>(expected[i]));
    }
}
#include "Logger.h"
#include "doctest.h"
#include <vector>

// Logger that captures log messages for verification
class SpyLogger : public Logger {
public:
    SpyLogger(LogLevel verbosity)
        : Logger(verbosity)
    {
    }

    std::vector<std::string> messages;

protected:
    void _log(std::string_view msg) const override
    {
        // Cast away const to store messages (acceptable for testing)
        const_cast<SpyLogger*>(this)->messages.push_back(std::string(msg));
    }
};

TEST_CASE("SpyLogger captures messages")
{
    SpyLogger logger(Logger::LogLevel::INFO);

    SUBCASE("INFO level logs all messages")
    {
        logger.set_verbosity(Logger::LogLevel::INFO);
        logger.info("info message");
        logger.warning("warning message");
        logger.error("error message");

        CHECK(logger.messages.size() == 3); // Each call logs one full message
        CHECK(logger.messages[0] == "INFO: info message");
        CHECK(logger.messages[1] == "WARNING: warning message");
        CHECK(logger.messages[2] == "ERROR: error message");
    }

    SUBCASE("WARNING level filters INFO")
    {
        logger.set_verbosity(Logger::LogLevel::WARNING);
        logger.messages.clear();

        logger.info("info message");
        logger.warning("warning message");
        logger.error("error message");

        CHECK(logger.messages.size() == 2); // Only warning and error
        CHECK(logger.messages[0] == "WARNING: warning message");
        CHECK(logger.messages[1] == "ERROR: error message");
    }

    SUBCASE("ERROR level filters INFO and WARNING")
    {
        logger.set_verbosity(Logger::LogLevel::ERROR);
        logger.messages.clear();

        logger.info("info message");
        logger.warning("warning message");
        logger.error("error message");

        CHECK(logger.messages.size() == 1); // Only error
        CHECK(logger.messages[0] == "ERROR: error message");
    }

    SUBCASE("SILENT level logs nothing")
    {
        logger.set_verbosity(Logger::LogLevel::SILENT);
        logger.messages.clear();

        logger.info("info message");
        logger.warning("warning message");
        logger.error("error message");

        CHECK(logger.messages.empty());
    }

    SUBCASE("Get verbosity returns current level")
    {
        logger.set_verbosity(Logger::LogLevel::WARNING);
        CHECK(logger.get_verbosity() == Logger::LogLevel::WARNING);

        logger.set_verbosity(Logger::LogLevel::ERROR);
        CHECK(logger.get_verbosity() == Logger::LogLevel::ERROR);
    }
}

TEST_CASE("Logger with error codes")
{
    SpyLogger logger(Logger::LogLevel::INFO);

    logger.error("Test error (%d)", static_cast<int>(utils::ErrorCode::NETWORK_RESPONSE_NOT_OK_ERROR));

    // Should log "ERROR: Test error (20)"
    CHECK(logger.messages.size() == 1);
    CHECK(logger.messages[0] == "ERROR: Test error (20)");
}
