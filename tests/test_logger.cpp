#include "Logger.h"
#include "doctest.h"
#include "utils.h"

#include <string>
#include <vector>

// Logger that captures log messages for verification
class SpyLogger : public Logger {
public:
    SpyLogger(LogLevel verbosity)
        : Logger(verbosity)
    {
    }

    mutable std::vector<std::string> messages;

    // Override _log to capture messages
    void _log(std::string_view msg) const override { messages.push_back(std::string(msg)); }
};

TEST_CASE("Logger verbosity filtering")
{
    SpyLogger logger(Logger::LogLevel::INFO);

    SUBCASE("INFO level logs all messages")
    {
        logger.set_verbosity(Logger::LogLevel::INFO);
        logger.info("info message");
        logger.warning("warning message");
        logger.error("error message");

        CHECK(logger.messages.size() == 3);
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

        CHECK(logger.messages.size() == 2);
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

        CHECK(logger.messages.size() == 1);
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

    CHECK(logger.messages.size() == 1);
    CHECK(logger.messages[0] == "ERROR: Test error (20)");
}
