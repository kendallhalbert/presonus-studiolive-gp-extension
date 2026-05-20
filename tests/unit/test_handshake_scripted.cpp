#include <gtest/gtest.h>

#include <memory>
#include <string_view>

#include "FixtureUtil.h"
#include "protocol/ConnectionHandshake.h"
#include "protocol/JmPacket.h"
#include "protocol/MixerConnection.h"
#include "transport/ScriptedTransport.h"

namespace
{

TEST(HandshakeScripted, SubscribeThenChunkedZbAndSubscriptionReply)
{
    auto transport = std::make_unique<presonus::studiolive::gpext::transport::ScriptedTransport>();
    auto *transportPtr = transport.get();

    presonus::studiolive::gpext::protocol::MixerConnection connection(std::move(transport));

    presonus::studiolive::gpext::protocol::ConnectionHandshake handshake;
    connection.setSessionPacketCallback(
        [&handshake](const presonus::studiolive::gpext::protocol::SessionPacket &packet) {
            handshake.onSessionPacket(packet);
        });
    connection.setJsonMessageCallback([&handshake](std::string_view json) {
        handshake.onJmJson(json);
    });
    connection.setKeepAliveEnabled(false);

    ASSERT_TRUE(connection.connect("10.0.0.14", 53000));
    ASSERT_TRUE(connection.sendRaw(presonus::studiolive::gpext::protocol::createSubscribePacket()));

    EXPECT_FALSE(transportPtr->written().empty());

    for (const char *chunkPath :
         {"03-handshake-zb-chunked/000.bin", "03-handshake-zb-chunked/001.bin"})
    {
        transportPtr->enqueueInbound(readFixtureBin(chunkPath));
    }

    transportPtr->enqueueInbound(
        readFixtureBin("04-jm-subscription-reply/04-jm-subscription-reply.bin"));
    transportPtr->deliverInbound();

    for (int i = 0; i < 20 && !handshake.progress().complete(); ++i)
    {
        connection.poll();
    }

    EXPECT_TRUE(handshake.progress().zlibStateReceived);
    EXPECT_TRUE(handshake.progress().subscriptionReplyReceived);
    EXPECT_TRUE(handshake.progress().complete());
}

} // namespace
