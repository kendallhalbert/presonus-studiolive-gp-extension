#include <gtest/gtest.h>

#include "FixtureUtil.h"
#include "protocol/SessionPacketDecoder.h"
#include "protocol/ZlibState.h"

namespace
{

TEST(SessionPacketDecoder, ChunkedCkYieldsZlibState)
{
    presonus::studiolive::gpext::protocol::SessionPacketDecoder decoder;
    std::optional<presonus::studiolive::gpext::protocol::SessionPacket> result;

    for (const char *chunkPath : {"03-handshake-zb-chunked/000.bin", "03-handshake-zb-chunked/001.bin"})
    {
        const auto frame = readFixtureBin(chunkPath);
        result = decoder.feed(frame);
    }

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->wire.messageCode, "CK");
    ASSERT_TRUE(std::holds_alternative<presonus::studiolive::gpext::protocol::ZlibStateNode>(
        result->payload));

    const auto &state =
        std::get<presonus::studiolive::gpext::protocol::ZlibStateNode>(result->payload);
    const auto mixerName = presonus::studiolive::gpext::protocol::zlibStateStringAt(
        state, "global/mixer_name");
    ASSERT_TRUE(mixerName.has_value());
    EXPECT_EQ(*mixerName, "StudioLive 32R");
}

} // namespace
