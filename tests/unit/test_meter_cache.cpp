#include <gtest/gtest.h>

#include "protocol/LevlParser.h"
#include "state/MeterCache.h"

namespace
{

presonus::studiolive::gpext::protocol::LevlMessage makeMessage(int groupId, int channelIndex,
                                                               std::uint16_t raw)
{
    presonus::studiolive::gpext::protocol::LevlMessage message;
    message.levelsByGroup[groupId] =
        std::vector<std::uint16_t>(presonus::studiolive::gpext::protocol::kLevlChannelsPerGroup,
                                   0);
    message.levelsByGroup[groupId][static_cast<std::size_t>(channelIndex)] = raw;
    return message;
}

TEST(MeterCache, StoresAndReturnsPercent)
{
    presonus::studiolive::gpext::state::MeterCache cache;
    cache.apply(makeMessage(0, 0, 6553));

    const auto level = cache.levelPercent(0, 1);
    ASSERT_TRUE(level.has_value());
    EXPECT_NEAR(*level, 10.0, 0.05);
}

TEST(MeterCache, MissingChannelReturnsNullopt)
{
    presonus::studiolive::gpext::state::MeterCache cache;
    EXPECT_FALSE(cache.levelPercent(0, 1).has_value());
    EXPECT_FALSE(cache.levelPercent(9, 1).has_value());
    EXPECT_FALSE(cache.levelPercent(0, 0).has_value());
}

TEST(MeterCache, ClearRemovesData)
{
    presonus::studiolive::gpext::state::MeterCache cache;
    cache.apply(makeMessage(1, 3, 1000));
    ASSERT_TRUE(cache.hasData());
    cache.clear();
    EXPECT_FALSE(cache.hasData());
    EXPECT_FALSE(cache.levelPercent(1, 4).has_value());
}

} // namespace
