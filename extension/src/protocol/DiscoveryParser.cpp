#include "protocol/DiscoveryParser.h"

#include "protocol/MessageProtocol.h"

namespace presonus::studiolive::gpext::protocol
{

namespace
{

constexpr std::size_t kDiscoveryPayloadPrefixBytes = 20;

std::vector<std::string> splitNullSeparatedStrings(std::span<const std::uint8_t> bytes)
{
    std::vector<std::string> fragments;
    std::size_t start = 0;
    for (std::size_t i = 0; i < bytes.size(); ++i)
    {
        if (bytes[i] == 0)
        {
            fragments.emplace_back(reinterpret_cast<const char *>(bytes.data() + start),
                                   i - start);
            start = i + 1;
        }
    }
    return fragments;
}

} // namespace

std::optional<DiscoveredMixer> parseDiscoveryPacket(std::span<const std::uint8_t> packet,
                                                    const std::string &sourceHost)
{
    const auto parsed = analysePacket(packet, true);
    if (!parsed.has_value() || parsed->payload.size() < kDiscoveryPayloadPrefixBytes)
    {
        return std::nullopt;
    }

    const auto stringRegion = std::span<const std::uint8_t>(
        parsed->payload.data() + kDiscoveryPayloadPrefixBytes,
        parsed->payload.size() - kDiscoveryPayloadPrefixBytes);
    const auto fragments = splitNullSeparatedStrings(stringRegion);
    if (fragments.size() < 3 || fragments[2].empty())
    {
        return std::nullopt;
    }

    DiscoveredMixer mixer;
    mixer.host = sourceHost;
    mixer.name = fragments[0];
    mixer.serial = fragments[2];
    return mixer;
}

} // namespace presonus::studiolive::gpext::protocol
