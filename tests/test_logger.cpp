#include "Logger.h"
#include "doctest.h"
#include <sstream>
#include <vector>

// Mock logger that captures log messages
class MockLogger : public Logger {
public:
    MockLogger(LogLevel verbosity)
        : Logger(verbosity)
    {
    }

    std::vector<std::string> messages;

protected:
    void _log(std::string_view msg) const override
    {
        // Cast away const to store messages (acceptable for testing)
        const_cast<MockLogger*>(this)->messages.push_back(std::string(msg));
    }
};

TEST_CASE("Logger verbosity levels")
{
    MockLogger logger(Logger::LogLevel::INFO);

    SUBCASE("INFO level logs all messages")
    {
        logger.set_verbosity(Logger::LogLevel::INFO);
        logger.info("info message");
        logger.warning("warning message");
        logger.error("error message");

        CHECK(logger.messages.size() == 6); // Each call logs prefix + message
        CHECK(logger.messages[0] == "INFO: ");
        CHECK(logger.messages[1] == "info message");
        CHECK(logger.messages[2] == "WARNING: ");
        CHECK(logger.messages[3] == "warning message");
        CHECK(logger.messages[4] == "ERROR: ");
        CHECK(logger.messages[5] == "error message");
    }

    SUBCASE("WARNING level filters INFO")
    {
        logger.set_verbosity(Logger::LogLevel::WARNING);
        logger.messages.clear();

        logger.info("info message");
        logger.warning("warning message");
        logger.error("error message");

        CHECK(logger.messages.size() == 4); // Only warning and error
        CHECK(logger.messages[0] == "WARNING: ");
        CHECK(logger.messages[2] == "ERROR: ");
    }

    SUBCASE("ERROR level filters INFO and WARNING")
    {
        logger.set_verbosity(Logger::LogLevel::ERROR);
        logger.messages.clear();

        logger.info("info message");
        logger.warning("warning message");
        logger.error("error message");

        CHECK(logger.messages.size() == 2); // Only error
        CHECK(logger.messages[0] == "ERROR: ");
        CHECK(logger.messages[1] == "error message");
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
    MockLogger logger(Logger::LogLevel::INFO);

    logger.error("Test error", utils::ErrorCode::NETWORK_RESPONSE_NOT_OK_ERROR);

    // Should log "ERROR", " (20)", ": ", "Test error"
    CHECK(logger.messages.size() == 4);
    CHECK(logger.messages[0] == "ERROR");
    CHECK(logger.messages[1].find("20") != std::string::npos); // Error code 20
    CHECK(logger.messages[2] == ": ");
    CHECK(logger.messages[3] == "Test error");
}
