#include <gtest/gtest.h>

#include "protocol/DiscoveryParser.h"
#include "protocol/MessageProtocol.h"

namespace
{

std::vector<std::uint8_t> makeSampleDiscoveryPacket(const std::string &modelName,
                                                    const std::string &serial)
{
    std::vector<std::uint8_t> payload(20, 0);
    const auto appendString = [&payload](const std::string &text) {
        payload.insert(payload.end(), text.begin(), text.end());
        payload.push_back(0);
    };
    appendString(modelName);
    appendString("");
    appendString(serial);
    appendString(modelName);

    return presonus::studiolive::gpext::protocol::createPacket("DA", payload, 0x65, 0x00);
}

TEST(DiscoveryParser, ParsesDocumentedLayout)
{
    const auto packet = makeSampleDiscoveryPacket("StudioLive 32R", "RA3E18090022");
    const auto mixer =
        presonus::studiolive::gpext::protocol::parseDiscoveryPacket(packet, "10.0.0.14");
    ASSERT_TRUE(mixer.has_value());
    EXPECT_EQ(mixer->host, "10.0.0.14");
    EXPECT_EQ(mixer->name, "StudioLive 32R");
    EXPECT_EQ(mixer->serial, "RA3E18090022");
    EXPECT_EQ(mixer->tcpPort, 53000);
}

TEST(DiscoveryParser, RejectsTruncatedPacket)
{
    const auto packet = makeSampleDiscoveryPacket("StudioLive 32R", "RA3E18090022");
    const auto truncated = std::span<const std::uint8_t>(packet.data(), 10);
    EXPECT_FALSE(presonus::studiolive::gpext::protocol::parseDiscoveryPacket(truncated,
                                                                             "10.0.0.14")
                     .has_value());
}

} // namespace
