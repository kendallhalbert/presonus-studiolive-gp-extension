#include <gtest/gtest.h>

#include <variant>

#include "FixtureUtil.h"
#include "protocol/PacketParser.h"
#include "protocol/ParamKeys.h"
#include "protocol/PvEncoder.h"
#include "state/KvCache.h"

namespace
{

TEST(LineControls, VolumePvUpdatesCache)
{
    const auto packet = presonus::studiolive::gpext::protocol::createPvPacket(
        presonus::studiolive::gpext::protocol::lineChannelVolumeKey(1), 0.5F);
    const auto parsed = presonus::studiolive::gpext::protocol::parseWirePacket(packet);
    ASSERT_TRUE(parsed.has_value());

    presonus::studiolive::gpext::state::KvCache cache;
    std::visit(
        [&](const auto &payload) {
            cache.apply(presonus::studiolive::gpext::protocol::SessionPacket{
                parsed->wire, payload});
        },
        parsed->payload);

    const auto volume = cache.doubleKey("line/ch1/volume");
    ASSERT_TRUE(volume.has_value());
    EXPECT_NEAR(*volume, 0.5, 0.001);
}

TEST(LineControls, PanFixtureScalar)
{
    const auto bytes = readFixtureBin("10-pv-pan.bin");
    const auto parsed = presonus::studiolive::gpext::protocol::parseWirePacket(bytes);
    ASSERT_TRUE(parsed.has_value());

    presonus::studiolive::gpext::state::KvCache cache;
    std::visit(
        [&](const auto &payload) {
            cache.apply(presonus::studiolive::gpext::protocol::SessionPacket{
                parsed->wire, payload});
        },
        parsed->payload);

    const auto pan = cache.doubleKey("line/ch1/pan");
    ASSERT_TRUE(pan.has_value());
    EXPECT_NEAR(*pan, 0.25, 0.01);
}

} // namespace
