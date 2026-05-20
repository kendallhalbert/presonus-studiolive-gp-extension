#include <gtest/gtest.h>

#include "bridge/MockGpHost.h"

namespace
{

TEST(MockGpHost, RecordsWidgetWrites)
{
    presonus::studiolive::gpext::bridge::MockGpHost host;
    host.setWidget("Fader1", 0.5);
    EXPECT_TRUE(host.widgetExists("Fader1"));
    EXPECT_DOUBLE_EQ(host.getWidgetValue("Fader1"), 0.5);
    EXPECT_TRUE(host.setWidgetValue("Fader1", 0.75));
    ASSERT_EQ(host.widgetWrites().size(), 1u);
    EXPECT_EQ(host.widgetWrites()[0].first, "Fader1");
    EXPECT_DOUBLE_EQ(host.widgetWrites()[0].second, 0.75);
}

} // namespace
