#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <vector>

#include "protocol/MessageProtocol.h"
#include "protocol/PvParser.h"

namespace
{

std::vector<std::uint8_t> readFixtureBin(const char *relativePath)
{
    const std::filesystem::path path =
        std::filesystem::path(PSL_FIXTURE_ROOT) / relativePath;
    std::ifstream in(path, std::ios::binary);
    EXPECT_TRUE(in.is_open()) << path.string();
    return std::vector<std::uint8_t>(std::istreambuf_iterator<char>(in), {});
}

TEST(MessageProtocol, AnalysePvMuteFixture)
{
    const auto bytes = readFixtureBin("05-pv-bool-mute.bin");
    ASSERT_EQ(bytes.size(), 32u);

    const auto parsed = presonus::studiolive::gpext::protocol::analysePacket(bytes);
    ASSERT_TRUE(parsed.has_value());
    EXPECT_EQ(parsed->messageCode, "PV");

    const auto pv = presonus::studiolive::gpext::protocol::parsePvPayload(parsed->payload);
    ASSERT_TRUE(pv.has_value());
    EXPECT_EQ(pv->key, "line/ch1/mute");

    const auto value = presonus::studiolive::gpext::protocol::pvPayloadFloat(pv->partB);
    ASSERT_TRUE(value.has_value());
    EXPECT_FLOAT_EQ(*value, 1.0F);
}

TEST(MessageProtocol, RoundTripPreservesPayload)
{
    const auto original = readFixtureBin("10-pv-pan.bin");
    const auto parsed = presonus::studiolive::gpext::protocol::analysePacket(original);
    ASSERT_TRUE(parsed.has_value());

    const auto crafted = presonus::studiolive::gpext::protocol::createPacket(
        parsed->messageCode, parsed->payload, parsed->customA, parsed->customB);
    EXPECT_EQ(crafted, original);
}

} // namespace
