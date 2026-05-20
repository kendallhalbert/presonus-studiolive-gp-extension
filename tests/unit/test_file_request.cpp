#include <gtest/gtest.h>

#include "protocol/FileRequest.h"
#include "protocol/MessageProtocol.h"

namespace
{

TEST(FileRequest, ListProjectsMatchesSessionCapture)
{
    const auto packet = presonus::studiolive::gpext::protocol::createFileListRequestPacket(
        0x688A, "presets/proj");

    const auto wire = presonus::studiolive::gpext::protocol::analysePacket(packet);
    ASSERT_TRUE(wire.has_value());
    EXPECT_EQ(wire->messageCode, "FR");
    ASSERT_GE(wire->payload.size(), 8u);
    EXPECT_EQ(wire->payload[0], 0x8A);
    EXPECT_EQ(wire->payload[1], 0x68);

    const std::string text(reinterpret_cast<const char *>(wire->payload.data() + 2),
                           wire->payload.size() - 2);
    EXPECT_NE(text.find("Listpresets/proj"), std::string::npos);
}

TEST(FileRequest, OpenScenePath)
{
    const auto packet = presonus::studiolive::gpext::protocol::createFileOpenRequestPacket(
        0x1234, "presets/proj/01.West End Girls.proj/Scene1.scene");

    const auto wire = presonus::studiolive::gpext::protocol::analysePacket(packet);
    ASSERT_TRUE(wire.has_value());
    EXPECT_EQ(wire->messageCode, "FR");

    const std::string text(reinterpret_cast<const char *>(wire->payload.data() + 2),
                           wire->payload.size() - 2);
    EXPECT_NE(text.find("Openpresets/proj/01.West End Girls.proj/Scene1.scene"),
              std::string::npos);
}

} // namespace
