#include <gtest/gtest.h>

#include "bridge/MockGpHost.h"
#include "bridge/WidgetBindingRegistry.h"
#include "mixer/MixerService.h"

namespace
{

TEST(WidgetBindingRegistry, WidgetToMixerLevelLinear)
{
    presonus::studiolive::gpext::bridge::MockGpHost host;
    presonus::studiolive::gpext::bridge::Logger logger;
    presonus::studiolive::gpext::mixer::MixerService mixer(logger);

    host.setWidget("Fader1", 0.5);

    presonus::studiolive::gpext::bridge::WidgetBindingRegistry registry;
    ASSERT_TRUE(registry.bindLineLevelLinear(
        host, "Fader1", 1,
        presonus::studiolive::gpext::bridge::WidgetDirection::WidgetToMixer));

    registry.onWidgetValueChanged(host, mixer, "Fader1", 0.75);
    EXPECT_FALSE(mixer.isConnected());
}

TEST(WidgetBindingRegistry, SuppressesEchoAfterPush)
{
    presonus::studiolive::gpext::bridge::MockGpHost host;
    presonus::studiolive::gpext::bridge::Logger logger;
    presonus::studiolive::gpext::mixer::MixerService mixer(logger);

    host.setWidget("Mute1", 0.0);

    presonus::studiolive::gpext::bridge::WidgetBindingRegistry registry;
    ASSERT_TRUE(registry.bindLineMute(
        host, "Mute1", 1,
        presonus::studiolive::gpext::bridge::WidgetDirection::Both));

    registry.pollMixerToWidgets(host, mixer);
    const auto writesBefore = host.widgetWrites().size();

    registry.onWidgetValueChanged(host, mixer, "Mute1", host.getWidgetValue("Mute1"));
    EXPECT_EQ(host.widgetWrites().size(), writesBefore);
}

} // namespace
