#include <gtest/gtest.h>

#include <cstring>

#include "protocol/JmPacket.h"
#include "protocol/LevlParser.h"
#include "protocol/MessageProtocol.h"

namespace
{

std::vector<std::uint8_t> buildSyntheticLevlPacket()
{
    std::vector<std::uint8_t> packet(20 + presonus::studiolive::gpext::protocol::kLevlDataSize, 0);
    std::memcpy(packet.data(), presonus::studiolive::gpext::protocol::kPacketHeader,
                presonus::studiolive::gpext::protocol::kPacketHeaderSize);
    packet[4] = static_cast<std::uint8_t>((packet.size() - 6) & 0xFF);
    packet[5] = static_cast<std::uint8_t>((packet.size() - 6) >> 8);
    packet[6] = 'M';
    packet[7] = 'S';
    std::memcpy(packet.data() + 12, "levl", 4);

    auto writeLevel = [&packet](std::size_t offset, std::uint16_t value) {
        packet[20 + offset] = static_cast<std::uint8_t>(value & 0xFF);
        packet[20 + offset + 1] = static_cast<std::uint8_t>(value >> 8);
    };

    writeLevel(0, 1000);
    writeLevel(2, 2000);
    writeLevel(72, 3000);
    writeLevel(104, 4000);
    return packet;
}

TEST(LevlParser, ParsesSyntheticLevlFrame)
{
    const auto packet = buildSyntheticLevlPacket();
    const auto message = presonus::studiolive::gpext::protocol::parseLevlPacket(packet);
    ASSERT_TRUE(message.has_value());
    ASSERT_EQ(message->levelsByGroup.size(), 5u);
    EXPECT_EQ(message->levelsByGroup.at(0)[0], 1000);
    EXPECT_EQ(message->levelsByGroup.at(0)[1], 2000);
    EXPECT_EQ(message->levelsByGroup.at(1)[0], 3000);
    EXPECT_EQ(message->levelsByGroup.at(2)[0], 4000);
}

TEST(LevlParser, ParsesCompact567ByteStudioLiveFrame)
{
    auto packet = buildSyntheticLevlPacket();
    packet.resize(567);
    packet[4] = static_cast<std::uint8_t>((packet.size() - 6) & 0xFF);
    packet[5] = static_cast<std::uint8_t>((packet.size() - 6) >> 8);

    const auto message = presonus::studiolive::gpext::protocol::parseLevlPacket(packet);
    ASSERT_TRUE(message.has_value());
    EXPECT_EQ(message->levelsByGroup.at(0)[0], 1000);
    EXPECT_EQ(message->levelsByGroup.at(1)[0], 3000);
}

TEST(LevlParser, RejectsNonLevlPacket)
{
    auto packet = buildSyntheticLevlPacket();
    packet[12] = 'f';
    packet[13] = 'd';
    packet[14] = 'r';
    packet[15] = 's';
    EXPECT_FALSE(presonus::studiolive::gpext::protocol::parseLevlPacket(packet).has_value());
}

TEST(LevlParser, AcceptsFdMessageCode)
{
    auto packet = buildSyntheticLevlPacket();
    packet[6] = 'F';
    packet[7] = 'D';
    const auto message = presonus::studiolive::gpext::protocol::parseLevlPacket(packet);
    ASSERT_TRUE(message.has_value());
    EXPECT_EQ(message->levelsByGroup.at(0)[0], 1000);
}

TEST(JmPacket, HelloPacketCarriesUdpPort)
{
    const auto packet = presonus::studiolive::gpext::protocol::createHelloPacket(52704);
    const auto wire = presonus::studiolive::gpext::protocol::analysePacket(packet);
    ASSERT_TRUE(wire.has_value());
    EXPECT_EQ(wire->messageCode, "UM");
    ASSERT_EQ(wire->payload.size(), 2u);
    EXPECT_EQ(wire->payload[0], 0xE0);
    EXPECT_EQ(wire->payload[1], 0xCD);
    EXPECT_EQ(wire->customA, 0x00);
}

} // namespace
