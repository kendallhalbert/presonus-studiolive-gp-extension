#include <gtest/gtest.h>

#include <vector>

#include "FixtureUtil.h"
#include "protocol/DataClient.h"
#include "protocol/MessageProtocol.h"

namespace
{

TEST(DataClient, ReassemblesChunkedTcpStream)
{
    const auto fullPacket = readFixtureBin("05-pv-bool-mute.bin");
    ASSERT_GT(fullPacket.size(), 16u);

    std::vector<std::vector<std::uint8_t>> packets;
    presonus::studiolive::gpext::protocol::DataClient client(
        [&](std::vector<std::uint8_t> packet) { packets.push_back(std::move(packet)); });

    const std::size_t splitAt = 10;
    client.feed(std::span<const std::uint8_t>(fullPacket.data(), splitAt));
    EXPECT_TRUE(packets.empty());

    client.feed(std::span<const std::uint8_t>(fullPacket.data() + splitAt,
                                              fullPacket.size() - splitAt));
    ASSERT_EQ(packets.size(), 1u);
    EXPECT_EQ(packets[0], fullPacket);

    const auto parsed = presonus::studiolive::gpext::protocol::analysePacket(packets[0]);
    ASSERT_TRUE(parsed.has_value());
    EXPECT_EQ(parsed->messageCode, "PV");
}

TEST(DataClient, DeliversBackToBackPackets)
{
    const auto first = readFixtureBin("05-pv-bool-mute.bin");
    const auto second = readFixtureBin("10-pv-pan.bin");

    std::vector<std::vector<std::uint8_t>> packets;
    presonus::studiolive::gpext::protocol::DataClient client(
        [&](std::vector<std::uint8_t> packet) { packets.push_back(std::move(packet)); });

    std::vector<std::uint8_t> stream;
    stream.insert(stream.end(), first.begin(), first.end());
    stream.insert(stream.end(), second.begin(), second.end());
    client.feed(stream);

    ASSERT_EQ(packets.size(), 2u);
    EXPECT_EQ(packets[0], first);
    EXPECT_EQ(packets[1], second);
}

} // namespace
