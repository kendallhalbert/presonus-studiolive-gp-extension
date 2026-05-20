#include <gtest/gtest.h>

#include "FixtureUtil.h"
#include "protocol/CkAssembler.h"
#include "protocol/MessageProtocol.h"
#include "protocol/Ubjson.h"
#include "protocol/ZlibInflate.h"
#include "protocol/ZlibState.h"
#include "protocol/ZbParser.h"

namespace
{

TEST(ZlibHandshake, ChunkedCkRebuildsMixerMetadata)
{
    presonus::studiolive::gpext::protocol::CkAssembler assembler;
    std::optional<std::vector<std::uint8_t>> zlibBlob;

    for (const char *chunkPath : {"03-handshake-zb-chunked/000.bin", "03-handshake-zb-chunked/001.bin"})
    {
        const auto frame = readFixtureBin(chunkPath);
        const auto wire = presonus::studiolive::gpext::protocol::analysePacket(frame);
        ASSERT_TRUE(wire.has_value());
        EXPECT_EQ(wire->messageCode, "CK");
        zlibBlob = assembler.addChunk(wire->payload);
    }

    ASSERT_TRUE(zlibBlob.has_value());

    const auto inflated = presonus::studiolive::gpext::protocol::zlibInflate(*zlibBlob);
    ASSERT_TRUE(inflated.has_value()) << "zlib inflate failed";
    EXPECT_GT(inflated->size(), 1000u);
    EXPECT_EQ((*inflated)[0], 0x7B);

    const auto root = presonus::studiolive::gpext::protocol::parseUbjsonObject(*inflated);
    ASSERT_TRUE(root.has_value()) << "UBJSON parse failed";
    ASSERT_TRUE(root->contains("children"));

    const auto state = presonus::studiolive::gpext::protocol::parseZlibState(*root);
    ASSERT_TRUE(state.has_value()) << "ZlibState parse failed";

    // Synchronize payload: children.global.values (no "internal" wrapper; see snapshot-state.json).
    const auto mixerName =
        presonus::studiolive::gpext::protocol::zlibStateStringAt(*state, "global/mixer_name");
    ASSERT_TRUE(mixerName.has_value());
    EXPECT_EQ(*mixerName, "StudioLive 32R");

    const auto serial =
        presonus::studiolive::gpext::protocol::zlibStateStringAt(*state, "global/mixer_serial");
    ASSERT_TRUE(serial.has_value());
    EXPECT_EQ(*serial, "RA3E18090022");
}

} // namespace
