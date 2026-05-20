#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace presonus::studiolive::gpext::protocol
{

/// Linear fader position 0..100 (JS divides uint16 LE by 655.35).
using FaderLevels = std::vector<double>;

struct MsMessage
{
    std::map<std::string, FaderLevels> levelsByChannelType;
};

std::optional<MsMessage> parseMsPayload(std::span<const std::uint8_t> payload);

} // namespace presonus::studiolive::gpext::protocol
