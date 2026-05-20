#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <sstream>

#include "bridge/FileLogSink.h"
#include "bridge/LogLevelUtil.h"

namespace
{

TEST(FileLogSink, AppendsTimestampedLines)
{
    const auto path =
        std::filesystem::temp_directory_path() / "psl_test_extension.log";
    std::error_code ec;
    std::filesystem::remove(path, ec);

    presonus::studiolive::gpext::bridge::FileLogSink sink(path);
    sink.writeSessionBanner("test");
    sink.write(presonus::studiolive::gpext::bridge::LogLevel::Info, "hello");

    std::ifstream in(path);
    ASSERT_TRUE(in.good());
    std::ostringstream contents;
    contents << in.rdbuf();
    const std::string text = contents.str();
    EXPECT_NE(text.find("PreSonus StudioLive test"), std::string::npos);
    EXPECT_NE(text.find("[INFO]"), std::string::npos);
    EXPECT_NE(text.find("hello"), std::string::npos);

    std::filesystem::remove(path, ec);
}

TEST(FileLogSink, ParseLogLevelNames)
{
    using presonus::studiolive::gpext::bridge::LogLevel;
    using presonus::studiolive::gpext::bridge::parseLogLevel;

    ASSERT_TRUE(parseLogLevel("debug").has_value());
    EXPECT_EQ(*parseLogLevel("debug"), LogLevel::Debug);
    EXPECT_EQ(*parseLogLevel("info"), LogLevel::Info);
    EXPECT_EQ(*parseLogLevel("warn"), LogLevel::Warn);
    EXPECT_FALSE(parseLogLevel("verbose").has_value());
}

} // namespace
