#pragma once

#include <optional>
#include <span>
#include <string>

namespace presonus::studiolive::gpext::protocol
{

struct PcMessage
{
    std::string key;
    /// First four value bytes as lowercase hex (matches JS `fromPC` color transformer).
    std::string valueHex;
};

std::optional<PcMessage> parsePcPayload(std::span<const std::uint8_t> payload);

} // namespace presonus::studiolive::gpext::protocol
