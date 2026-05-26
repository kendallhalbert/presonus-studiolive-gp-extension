#include <gtest/gtest.h>

#include "bridge/DisplayNameUtil.h"

TEST(DisplayNameUtil, SceneFileStripsPrefixAndExtension)
{
    EXPECT_EQ(presonus::studiolive::gpext::bridge::presetFileDisplayName("03.Mix Reference.scn"),
              "Mix Reference");
}

TEST(DisplayNameUtil, ProjectFileStripsPrefixAndExtension)
{
    EXPECT_EQ(
        presonus::studiolive::gpext::bridge::presetFileDisplayName("01.West End Girls.proj"),
        "West End Girls");
}

TEST(DisplayNameUtil, LeavesNameWithoutNumericPrefixUntouched)
{
    EXPECT_EQ(presonus::studiolive::gpext::bridge::presetFileDisplayName("Live Performance.scn"),
              "Live Performance");
}

TEST(DisplayNameUtil, EmptyInputReturnsEmpty)
{
    EXPECT_TRUE(presonus::studiolive::gpext::bridge::presetFileDisplayName("").empty());
}

TEST(DisplayNameUtil, DoesNotStripNonNumericPrefix)
{
    EXPECT_EQ(presonus::studiolive::gpext::bridge::presetFileDisplayName("A1.Test.scn"), "A1.Test");
}
