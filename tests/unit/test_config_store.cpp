#include <gtest/gtest.h>

#include <filesystem>

#include "bridge/ConfigStore.h"

namespace
{

TEST(ConfigStore, SaveAndLoadLastHost)
{
    const auto path = presonus::studiolive::gpext::bridge::ConfigStore::configFilePath();
    std::error_code ec;
    std::filesystem::remove(path, ec);

    presonus::studiolive::gpext::bridge::ConfigStore store;
    store.setLastHost("192.168.1.50");
    ASSERT_TRUE(store.save());

    presonus::studiolive::gpext::bridge::ConfigStore reloaded;
    ASSERT_TRUE(reloaded.load());
    ASSERT_TRUE(reloaded.lastHost().has_value());
    EXPECT_EQ(*reloaded.lastHost(), "192.168.1.50");

    std::filesystem::remove(path, ec);
}

TEST(ConfigStore, SaveAndLoadSerialAndMixerName)
{
    const auto path = presonus::studiolive::gpext::bridge::ConfigStore::configFilePath();
    std::error_code ec;
    std::filesystem::remove(path, ec);

    presonus::studiolive::gpext::bridge::ConfigStore store;
    store.setLastHost("10.0.0.14");
    store.setLastSerial("RA3E18090022");
    store.setLastMixerName("StudioLive 32R");
    ASSERT_TRUE(store.save());

    presonus::studiolive::gpext::bridge::ConfigStore reloaded;
    ASSERT_TRUE(reloaded.load());
    ASSERT_TRUE(reloaded.lastHost().has_value());
    ASSERT_TRUE(reloaded.lastSerial().has_value());
    ASSERT_TRUE(reloaded.lastMixerName().has_value());
    EXPECT_EQ(*reloaded.lastHost(), "10.0.0.14");
    EXPECT_EQ(*reloaded.lastSerial(), "RA3E18090022");
    EXPECT_EQ(*reloaded.lastMixerName(), "StudioLive 32R");

    std::filesystem::remove(path, ec);
}

} // namespace
