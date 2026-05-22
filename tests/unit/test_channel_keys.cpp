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

TEST(ChannelKeys, ReturnMainMixKeys)
{
    const auto target = presonus::studiolive::gpext::protocol::parseChannelTarget(
        "RETURN", 2, "", 0);
    ASSERT_TRUE(target.has_value());
    EXPECT_EQ(presonus::studiolive::gpext::protocol::levelPvKey(*target), "return/ch2/volume");
    EXPECT_EQ(presonus::studiolive::gpext::protocol::mutePvKey(*target), "return/ch2/mute");
    EXPECT_EQ(presonus::studiolive::gpext::protocol::soloPvKey(*target), "return/ch2/solo");
}

TEST(ChannelKeys, DcaMainMixKeys)
{
    const auto target = presonus::studiolive::gpext::protocol::parseChannelTarget(
        "DCA", 4, "", 0);
    ASSERT_TRUE(target.has_value());
    EXPECT_EQ(presonus::studiolive::gpext::protocol::levelPvKey(*target),
              "filtergroup/ch4/volume");
    EXPECT_EQ(presonus::studiolive::gpext::protocol::panPvKey(*target), "filtergroup/ch4/pan");
}

TEST(ChannelKeys, SubMainMixKeys)
{
    const auto target =
        presonus::studiolive::gpext::protocol::parseChannelTarget("SUB", 1, "", 0);
    ASSERT_TRUE(target.has_value());
    EXPECT_EQ(presonus::studiolive::gpext::protocol::levelPvKey(*target), "sub/ch1/volume");
}

TEST(ChannelKeys, MainForcesChannelOne)
{
    const auto target =
        presonus::studiolive::gpext::protocol::parseChannelTarget("MAIN", 99, "", 0);
    ASSERT_TRUE(target.has_value());
    EXPECT_EQ(target->channel, 1);
    EXPECT_EQ(presonus::studiolive::gpext::protocol::levelPvKey(*target), "main/ch1/volume");
}

TEST(ChannelKeys, AuxBusMainMixKeys)
{
    const auto target =
        presonus::studiolive::gpext::protocol::parseChannelTarget("AUX", 3, "", 0);
    ASSERT_TRUE(target.has_value());
    EXPECT_EQ(presonus::studiolive::gpext::protocol::levelPvKey(*target), "aux/ch3/volume");
}

} // namespace
