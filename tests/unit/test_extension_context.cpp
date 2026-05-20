#include <gtest/gtest.h>

#include "bridge/Dispatcher.h"
#include "bridge/ExtensionContext.h"
#include "bridge/Logger.h"
#include "bridge/MockGpHost.h"

namespace
{

TEST(ExtensionContext, DrainRunsPostedTasks)
{
    presonus::studiolive::gpext::bridge::MockGpHost host;
    presonus::studiolive::gpext::bridge::Dispatcher dispatcher;
    presonus::studiolive::gpext::bridge::Logger logger;
    presonus::studiolive::gpext::bridge::ExtensionContext ctx(host, dispatcher, logger);
    presonus::studiolive::gpext::bridge::ExtensionContext::setInstance(&ctx);

    int ran = 0;
    ASSERT_TRUE(dispatcher.post([&ran] { ++ran; }));
    ctx.drainGpTasks();
    EXPECT_EQ(ran, 1);

    presonus::studiolive::gpext::bridge::ExtensionContext::setInstance(nullptr);
}

} // namespace
