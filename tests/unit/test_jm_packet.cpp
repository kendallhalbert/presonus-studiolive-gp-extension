#include <gtest/gtest.h>

#include "protocol/ConnectionHandshake.h"
#include "protocol/JmPacket.h"
#include "protocol/MessageProtocol.h"
#include "protocol/SessionPacketDecoder.h"

namespace
{

TEST(JmPacket, SubscribePacketMatchesSessionCapture)
{
    const auto packet = presonus::studiolive::gpext::protocol::createSubscribePacket();
    const auto wire = presonus::studiolive::gpext::protocol::analysePacket(packet);
    ASSERT_TRUE(wire.has_value());
    EXPECT_EQ(wire->messageCode, "JM");
    EXPECT_EQ(packet.size(), 282u);
}

TEST(JmPacket, DetectsSubscriptionReply)
{
    EXPECT_TRUE(presonus::studiolive::gpext::protocol::isSubscriptionReplyJson(
        R"({"id": "SubscriptionReply"})"));
    EXPECT_FALSE(presonus::studiolive::gpext::protocol::isSubscriptionReplyJson(
        R"({"id": "UserLoggedIn"})"));
}

TEST(JmPacket, ExtractsJmJsonBody)
{
    const std::vector<std::uint8_t> payload = {
        0x1b, 0x00, 0x00, 0x00, '{', '"', 'i', 'd', '"', ':', ' ', '"', 'S', 'u', 'b',
        's',  'c',  'r',  'i',  'p', 't', 'i', 'o', 'n', 'R', 'e', 'p', 'l', 'y', '"',
        '}'};
    const auto json = presonus::studiolive::gpext::protocol::extractJmJson(payload);
    ASSERT_TRUE(json.has_value());
    EXPECT_TRUE(presonus::studiolive::gpext::protocol::isSubscriptionReplyJson(*json));
}

TEST(JmPacket, RestorePresetSceneMatchesJsApi)
{
    const auto packet = presonus::studiolive::gpext::protocol::createRestorePresetPacket(
        "presets/proj/01.West End Girls.proj/01.Live Performance.scn");
    const auto wire = presonus::studiolive::gpext::protocol::analysePacket(packet);
    ASSERT_TRUE(wire.has_value());
    EXPECT_EQ(wire->messageCode, "JM");

    const auto json = presonus::studiolive::gpext::protocol::extractJmJson(wire->payload);
    ASSERT_TRUE(json.has_value());
    EXPECT_NE(json->find("RestorePreset"), std::string::npos);
    EXPECT_NE(json->find("01.Live Performance.scn"), std::string::npos);
    EXPECT_NE(json->find("presetTargetSlave"), std::string::npos);
}

TEST(ConnectionHandshake, CompletesOnZbAndSubscriptionReply)
{
    presonus::studiolive::gpext::protocol::ConnectionHandshake handshake;

    presonus::studiolive::gpext::protocol::SessionPacket packet;
    packet.wire.messageCode = "ZB";
    packet.payload = presonus::studiolive::gpext::protocol::ZlibStateNode{};
    handshake.onSessionPacket(packet);
    EXPECT_FALSE(handshake.progress().complete());

    handshake.onJmJson(R"({"id": "SubscriptionReply"})");
    EXPECT_TRUE(handshake.progress().complete());
}

} // namespace
