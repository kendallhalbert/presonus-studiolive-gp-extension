#include <gtest/gtest.h>

#include <memory>

#include "FixtureUtil.h"
#include "protocol/MixerConnection.h"
#include "transport/ScriptedTransport.h"

namespace
{

TEST(MixerConnection, DeframerDispatchesPvOverScriptedTransport)
{
    auto transport = std::make_unique<presonus::studiolive::gpext::transport::ScriptedTransport>();
    auto *transportPtr = transport.get();

    presonus::studiolive::gpext::protocol::MixerConnection connection(std::move(transport));
    ASSERT_TRUE(connection.connect("127.0.0.1", 53000));

    std::optional<presonus::studiolive::gpext::protocol::SessionPacket> received;
    connection.setSessionPacketCallback(
        [&](presonus::studiolive::gpext::protocol::SessionPacket packet) { received = std::move(packet); });

    transportPtr->enqueueInbound(readFixtureBin("05-pv-bool-mute.bin"));
    transportPtr->deliverInbound();

    ASSERT_TRUE(received.has_value());
    EXPECT_EQ(received->wire.messageCode, "PV");
    EXPECT_TRUE(std::holds_alternative<presonus::studiolive::gpext::protocol::PvMessage>(
        received->payload));
}

TEST(MixerConnection, ReassemblesFdListFromScriptedChunks)
{
    auto transport = std::make_unique<presonus::studiolive::gpext::transport::ScriptedTransport>();
    auto *transportPtr = transport.get();

    presonus::studiolive::gpext::protocol::MixerConnection connection(std::move(transport));
    ASSERT_TRUE(connection.connect("127.0.0.1", 53000));

    std::optional<presonus::studiolive::gpext::protocol::FdListResult> received;
    connection.setFdListCallback(
        [&](presonus::studiolive::gpext::protocol::FdListResult result) { received = std::move(result); });

    for (const char *name : {"000.bin", "002.bin"})
    {
        transportPtr->enqueueInbound(
            readFixtureBin((std::string("14-fd-projects-list/") + name).c_str()));
        transportPtr->deliverInbound();
        if (received.has_value())
        {
            break;
        }
    }

    ASSERT_TRUE(received.has_value());
    EXPECT_GT(received->json.size(), 100u);
    EXPECT_EQ(received->json[0], '{');
}

} // namespace
