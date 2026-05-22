#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include <string>

namespace presonus::studiolive::gpext::protocol
{

/// One mixer seen on UCNet UDP discovery (port 47809).
struct DiscoveredMixer
{
    std::string host;
    std::string name;
    std::string serial;
    std::uint16_t tcpPort{53000};
};

/// Parse a UDP discovery broadcast (source IP supplied separately).
std::optional<DiscoveredMixer> parseDiscoveryPacket(std::span<const std::uint8_t> packet,
                                                    const std::string &sourceHost);

} // namespace presonus::studiolive::gpext::protocol
