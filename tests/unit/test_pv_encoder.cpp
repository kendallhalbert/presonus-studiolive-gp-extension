#include <gtest/gtest.h>

#include "FixtureUtil.h"
#include "protocol/MessageProtocol.h"
#include "protocol/PvEncoder.h"
#include "protocol/PvParser.h"

namespace
{

TEST(PvEncoder, MuteOnMatchesCaptureSession)
{
    const auto packet =
        presonus::studiolive::gpext::protocol::createPvBoolPacket("line/ch1/mute", true);

    const auto wire = presonus::studiolive::gpext::protocol::analysePacket(packet);
    ASSERT_TRUE(wire.has_value());
    EXPECT_EQ(wire->messageCode, "PV");
    EXPECT_EQ(wire->customA, presonus::studiolive::gpext::protocol::kCByteA);
    EXPECT_EQ(wire->customB, presonus::studiolive::gpext::protocol::kCByteB);

    const auto pv = presonus::studiolive::gpext::protocol::parsePvPayload(wire->payload);
    ASSERT_TRUE(pv.has_value());
    EXPECT_EQ(pv->key, "line/ch1/mute");
    const auto value = presonus::studiolive::gpext::protocol::pvPayloadFloat(pv->partB);
    ASSERT_TRUE(value.has_value());
    EXPECT_FLOAT_EQ(*value, 1.0F);
    EXPECT_EQ(packet.size(), 32u);
}

TEST(PvEncoder, MuteOffUsesZeroFloat)
{
    const auto packet =
        presonus::studiolive::gpext::protocol::createPvBoolPacket("line/ch1/mute", false);
    const auto pv = presonus::studiolive::gpext::protocol::parsePvPayload(
        presonus::studiolive::gpext::protocol::analysePacket(packet)->payload);
    ASSERT_TRUE(pv.has_value());
    const auto value = presonus::studiolive::gpext::protocol::pvPayloadFloat(pv->partB);
    ASSERT_TRUE(value.has_value());
    EXPECT_FLOAT_EQ(*value, 0.0F);
}

TEST(PvEncoder, OutgoingUsesClientCustomBytesWithSamePayload)
{
    const auto incoming = readFixtureBin("05-pv-bool-mute.bin");
    const auto parsed = presonus::studiolive::gpext::protocol::analysePacket(incoming);
    ASSERT_TRUE(parsed.has_value());

    const auto outgoing = presonus::studiolive::gpext::protocol::createPvBoolPacket(
        "line/ch1/mute", true);
    const auto outWire = presonus::studiolive::gpext::protocol::analysePacket(outgoing);
    ASSERT_TRUE(outWire.has_value());

    EXPECT_EQ(outWire->messageCode, parsed->messageCode);
    EXPECT_EQ(outWire->payload, parsed->payload);
    EXPECT_EQ(outWire->customA, presonus::studiolive::gpext::protocol::kCByteA);
    EXPECT_EQ(outWire->customB, presonus::studiolive::gpext::protocol::kCByteB);
    EXPECT_NE(outWire->customA, parsed->customA);
}

} // namespace
