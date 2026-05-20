#include <gtest/gtest.h>

#include "FixtureUtil.h"
#include "protocol/KeepAlive.h"
#include "protocol/MessageProtocol.h"

namespace
{

TEST(KeepAlive, FrPacketMatchesSessionCapture)
{
    const auto packet = presonus::studiolive::gpext::protocol::createKeepAliveFrPacket(0x84E8);
    const auto wire = presonus::studiolive::gpext::protocol::analysePacket(packet);
    ASSERT_TRUE(wire.has_value());
    EXPECT_EQ(wire->messageCode, "FR");
    ASSERT_GE(wire->payload.size(), 8u);
    EXPECT_EQ(wire->payload[0], 0xE8);
    EXPECT_EQ(wire->payload[1], 0x84);
    EXPECT_EQ(std::string(reinterpret_cast<const char *>(wire->payload.data() + 2), 4),
              "Ftbr");
}

TEST(KeepAlive, KaPacketMatchesSessionCapture)
{
    const auto packet = presonus::studiolive::gpext::protocol::createKeepAliveKaPacket();

    const auto wire = presonus::studiolive::gpext::protocol::analysePacket(packet);
    ASSERT_TRUE(wire.has_value());
    EXPECT_EQ(wire->messageCode, "KA");
    EXPECT_TRUE(wire->payload.empty());
    EXPECT_EQ(packet.size(), 12u);
}

TEST(KeepAlive, PollSendsKaAndFr)
{
    std::vector<std::vector<std::uint8_t>> sent;
    presonus::studiolive::gpext::protocol::KeepAlive keepAlive(
        [&](std::vector<std::uint8_t> packet) { sent.push_back(std::move(packet)); },
        []() {});

    const auto now = std::chrono::steady_clock::now();
    keepAlive.poll(now);

    ASSERT_EQ(sent.size(), 2u);
    const auto ka = presonus::studiolive::gpext::protocol::analysePacket(sent[0]);
    const auto fr = presonus::studiolive::gpext::protocol::analysePacket(sent[1]);
    ASSERT_TRUE(ka.has_value());
    ASSERT_TRUE(fr.has_value());
    EXPECT_EQ(ka->messageCode, "KA");
    EXPECT_EQ(fr->messageCode, "FR");
}

TEST(KeepAlive, NotifyFdResponseClearsPendingProbe)
{
    bool timedOut = false;
    presonus::studiolive::gpext::protocol::KeepAlive keepAlive(
        [&](std::vector<std::uint8_t> /*packet*/) {},
        [&]() { timedOut = true; });

    const auto t0 = std::chrono::steady_clock::now();
    keepAlive.poll(t0);
    keepAlive.notifyFdResponse(0);

    keepAlive.poll(t0 + std::chrono::seconds(2));
    EXPECT_FALSE(timedOut);
}

} // namespace
