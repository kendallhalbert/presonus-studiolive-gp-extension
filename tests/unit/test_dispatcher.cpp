#include <gtest/gtest.h>

#include "bridge/Dispatcher.h"

namespace
{

TEST(Dispatcher, PostAndDrainRunsOnGpThread)
{
    presonus::studiolive::gpext::bridge::Dispatcher dispatcher;
    int counter = 0;
    ASSERT_TRUE(dispatcher.post([&counter] { ++counter; }));
    ASSERT_TRUE(dispatcher.post([&counter] { counter += 10; }));
    EXPECT_EQ(dispatcher.pendingCount(), 2u);
    dispatcher.drain();
    EXPECT_EQ(counter, 11);
    EXPECT_EQ(dispatcher.pendingCount(), 0u);
}

TEST(Dispatcher, RejectsWhenFull)
{
    presonus::studiolive::gpext::bridge::Dispatcher dispatcher(2);
    EXPECT_TRUE(dispatcher.post([] {}));
    EXPECT_TRUE(dispatcher.post([] {}));
    EXPECT_FALSE(dispatcher.post([] {}));
    EXPECT_EQ(dispatcher.pendingCount(), 2u);
}

} // namespace
