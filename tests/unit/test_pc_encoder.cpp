#include <gtest/gtest.h>

#include "FixtureUtil.h"
#include "protocol/MessageProtocol.h"
#include "protocol/PcEncoder.h"
#include "protocol/PcParser.h"

namespace
{

TEST(PcEncoder, ColorFixtureRoundTrip)
{
    const auto original = readFixtureBin("11-pc-color.bin");
    const auto parsed = presonus::studiolive::gpext::protocol::analysePacket(original);
    ASSERT_TRUE(parsed.has_value());

    const auto crafted =
        presonus::studiolive::gpext::protocol::createPcPacket("line/ch1/color", 0xFF, 0x88, 0x00);
    const auto outWire = presonus::studiolive::gpext::protocol::analysePacket(crafted);
    ASSERT_TRUE(outWire.has_value());
    EXPECT_EQ(outWire->messageCode, parsed->messageCode);
    EXPECT_EQ(outWire->payload, parsed->payload);
}

TEST(PcEncoder, ParseRgbHex)
{
    std::uint8_t r = 0;
    std::uint8_t g = 0;
    std::uint8_t b = 0;
    ASSERT_TRUE(presonus::studiolive::gpext::protocol::parseRgbHex("FF8800", r, g, b));
    EXPECT_EQ(r, 0xFF);
    EXPECT_EQ(g, 0x88);
    EXPECT_EQ(b, 0x00);
}

} // namespace
