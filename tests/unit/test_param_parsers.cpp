#include <gtest/gtest.h>

#include <variant>

#include "FixtureUtil.h"
#include "protocol/PacketParser.h"

namespace
{

TEST(ParamParsers, PcColorFixture)
{
    const auto bytes = readFixtureBin("11-pc-color.bin");
    const auto parsed = presonus::studiolive::gpext::protocol::parseWirePacket(bytes);
    ASSERT_TRUE(parsed.has_value());
    ASSERT_TRUE(std::holds_alternative<presonus::studiolive::gpext::protocol::PcMessage>(
        parsed->payload));

    const auto &pc = std::get<presonus::studiolive::gpext::protocol::PcMessage>(parsed->payload);
    EXPECT_EQ(pc.key, "line/ch1/color");
    EXPECT_NE(pc.valueHex.find("ff8800"), std::string::npos);
}

TEST(ParamParsers, PsChannelNameFixture)
{
    const auto bytes = readFixtureBin("12-ps-channel-name.bin");
    const auto parsed = presonus::studiolive::gpext::protocol::parseWirePacket(bytes);
    ASSERT_TRUE(parsed.has_value());
    ASSERT_TRUE(std::holds_alternative<presonus::studiolive::gpext::protocol::PsMessage>(
        parsed->payload));

    const auto &ps = std::get<presonus::studiolive::gpext::protocol::PsMessage>(parsed->payload);
    EXPECT_EQ(ps.key, "line/ch1/username");
    EXPECT_EQ(ps.value, "psl-capture");
}

TEST(ParamParsers, MsFaderSweepFirstFrame)
{
    const auto bytes = readFixtureBin("13-ms-fader-sweep/000.bin");
    const auto parsed = presonus::studiolive::gpext::protocol::parseWirePacket(bytes);
    ASSERT_TRUE(parsed.has_value());
    ASSERT_TRUE(std::holds_alternative<presonus::studiolive::gpext::protocol::MsMessage>(
        parsed->payload));

    const auto &ms = std::get<presonus::studiolive::gpext::protocol::MsMessage>(parsed->payload);
    const auto lineIt = ms.levelsByChannelType.find("LINE");
    ASSERT_NE(lineIt, ms.levelsByChannelType.end());
    ASSERT_GE(lineIt->second.size(), 1u);
    EXPECT_GE(lineIt->second[0], 0.0);
    EXPECT_LE(lineIt->second[0], 100.0);
}

} // namespace
