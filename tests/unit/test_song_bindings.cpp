#include <gtest/gtest.h>

#include "bridge/SongBindingTable.h"

TEST(SongBindingTable, BindsAndLooksUpSong)
{
    presonus::studiolive::gpext::bridge::SongBindingTable table;
    ASSERT_TRUE(table.bindSong(2, "01.West End Girls.proj", "01.Live Performance.scn"));

    const auto binding = table.lookupSong(2);
    ASSERT_TRUE(binding.has_value());
    EXPECT_EQ(binding->projectFile, "01.West End Girls.proj");
    EXPECT_EQ(binding->sceneFile, "01.Live Performance.scn");
}

TEST(SongBindingTable, BindsSongPartUnderCurrentSong)
{
    presonus::studiolive::gpext::bridge::SongBindingTable table;
    ASSERT_TRUE(
        table.bindSongPart(1, 0, "01.West End Girls.proj", "02._ Empty Location _.scn"));

    const auto binding = table.lookupSongPart(1, 0);
    ASSERT_TRUE(binding.has_value());
    EXPECT_EQ(binding->sceneFile, "02._ Empty Location _.scn");
}
