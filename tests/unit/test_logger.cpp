#include <gtest/gtest.h>

#include "bridge/Logger.h"

namespace
{

TEST(Logger, RespectsMinimumLevel)
{
    presonus::studiolive::gpext::bridge::Logger logger;
    std::vector<std::string> lines;
    logger.setSink([&lines](presonus::studiolive::gpext::bridge::LogLevel,
                            std::string_view message) { lines.emplace_back(message); });
    logger.setMinLevel(presonus::studiolive::gpext::bridge::LogLevel::Warn);
    logger.debug("skip");
    logger.warn("keep");
    logger.error("keep2");
    ASSERT_EQ(lines.size(), 2u);
    EXPECT_EQ(lines[0], "keep");
    EXPECT_EQ(lines[1], "keep2");
}

} // namespace
