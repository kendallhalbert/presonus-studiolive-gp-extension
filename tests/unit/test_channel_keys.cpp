#include <gtest/gtest.h>

#include "protocol/ChannelKeys.h"

namespace
{

TEST(ChannelKeys, LineMainVolumeKey)
{
    const auto target = presonus::studiolive::gpext::protocol::parseChannelTarget(
        "LINE", 1, "", 0);
    ASSERT_TRUE(target.has_value());
    EXPECT_EQ(presonus::studiolive::gpext::protocol::levelPvKey(*target), "line/ch1/volume");
    EXPECT_EQ(presonus::studiolive::gpext::protocol::mutePvKey(*target), "line/ch1/mute");
    EXPECT_FALSE(presonus::studiolive::gpext::protocol::sendMuteUsesInvertedAssign(*target));
}

TEST(ChannelKeys, LineAuxSendKeysMatchFixtures)
{
    const auto target = presonus::studiolive::gpext::protocol::parseChannelTarget(
        "LINE", 1, "AUX", 1);
    ASSERT_TRUE(target.has_value());
    EXPECT_EQ(presonus::studiolive::gpext::protocol::levelPvKey(*target), "line/ch1/aux1");
    EXPECT_EQ(presonus::studiolive::gpext::protocol::mutePvKey(*target),
              "line/ch1/assign_aux1");
    EXPECT_TRUE(presonus::studiolive::gpext::protocol::sendMuteUsesInvertedAssign(*target));
}

TEST(ChannelKeys, LineFxSendLevelKey)
{
    const auto target = presonus::studiolive::gpext::protocol::parseChannelTarget(
        "LINE", 1, "FX", 1);
    ASSERT_TRUE(target.has_value());
    EXPECT_EQ(presonus::studiolive::gpext::protocol::levelPvKey(*target), "line/ch1/FXA");
    EXPECT_EQ(presonus::studiolive::gpext::protocol::mutePvKey(*target), "line/ch1/assign_fx1");
}

TEST(ChannelKeys, ChannelPresetTarget)
{
    const auto target = presonus::studiolive::gpext::protocol::parseChannelTarget(
        "LINE", 3, "", 0);
    ASSERT_TRUE(target.has_value());
    EXPECT_EQ(presonus::studiolive::gpext::protocol::channelPresetTarget(*target), "line/ch3");
}

} // namespace
