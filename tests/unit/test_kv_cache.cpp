#include <gtest/gtest.h>

#include <variant>

#include "FixtureUtil.h"
#include "protocol/PacketParser.h"
#include "protocol/SessionPacketDecoder.h"
#include "state/KvCache.h"

namespace
{

TEST(KvCache, PvMuteFixture)
{
    presonus::studiolive::gpext::state::KvCache cache;
    const auto bytes = readFixtureBin("05-pv-bool-mute.bin");
    const auto parsed = presonus::studiolive::gpext::protocol::parseWirePacket(bytes);
    ASSERT_TRUE(parsed.has_value());

    std::visit(
        [&](const auto &payload) {
            cache.apply(presonus::studiolive::gpext::protocol::SessionPacket{
                parsed->wire, payload});
        },
        parsed->payload);

    const auto muted = cache.boolKey("line/ch1/mute");
    ASSERT_TRUE(muted.has_value());
    EXPECT_TRUE(*muted);
}

TEST(KvCache, MsFaderSweepStoresLineLevel)
{
    presonus::studiolive::gpext::state::KvCache cache;
    const auto bytes = readFixtureBin("13-ms-fader-sweep/000.bin");
    const auto parsed = presonus::studiolive::gpext::protocol::parseWirePacket(bytes);
    ASSERT_TRUE(parsed.has_value());

    std::visit(
        [&](const auto &payload) {
            cache.apply(presonus::studiolive::gpext::protocol::SessionPacket{
                parsed->wire, payload});
        },
        parsed->payload);

    const auto level = cache.doubleKey("line/ch1/level");
    ASSERT_TRUE(level.has_value());
    EXPECT_GE(*level, 0.0);
    EXPECT_LE(*level, 100.0);
}

TEST(KvCache, PsChannelNameFixture)
{
    presonus::studiolive::gpext::state::KvCache cache;
    const auto bytes = readFixtureBin("12-ps-channel-name.bin");
    const auto parsed = presonus::studiolive::gpext::protocol::parseWirePacket(bytes);
    ASSERT_TRUE(parsed.has_value());

    std::visit(
        [&](const auto &payload) {
            cache.apply(presonus::studiolive::gpext::protocol::SessionPacket{
                parsed->wire, payload});
        },
        parsed->payload);

    const auto name = cache.stringKey("line/ch1/username");
    ASSERT_TRUE(name.has_value());
    EXPECT_EQ(*name, "psl-capture");
}

TEST(KvCache, ZlibHandshakeImportsMixerName)
{
    presonus::studiolive::gpext::protocol::SessionPacketDecoder decoder;
    std::optional<presonus::studiolive::gpext::protocol::SessionPacket> result;

    for (const char *chunkPath : {"03-handshake-zb-chunked/000.bin", "03-handshake-zb-chunked/001.bin"})
    {
        const auto frame = readFixtureBin(chunkPath);
        result = decoder.feed(frame);
    }

    ASSERT_TRUE(result.has_value());
    presonus::studiolive::gpext::state::KvCache cache;
    cache.apply(*result);

    const auto mixerName = cache.stringKey("global/mixer_name");
    ASSERT_TRUE(mixerName.has_value());
    EXPECT_EQ(*mixerName, "StudioLive 32R");
}

TEST(KvCache, OptimisticBoolRoundTrip)
{
    presonus::studiolive::gpext::state::KvCache cache;
    cache.setBool("line/ch2/mute", true);
    EXPECT_TRUE(cache.boolKey("line/ch2/mute").value_or(false));
    cache.setBool("line/ch2/mute", false);
    EXPECT_FALSE(cache.boolKey("line/ch2/mute").value_or(true));
}

} // namespace
