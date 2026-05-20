#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace presonus::studiolive::gpext::protocol
{

struct PvMessage
{
    std::string key;
    std::vector<std::uint8_t> partA;
    std::vector<std::uint8_t> partB;
};

/// Parse the payload of a PV packet (bytes after the 12-byte UCNet prefix).
std::optional<PvMessage> parsePvPayload(std::span<const std::uint8_t> payload);

/// Read the first float in partB (bool PV values use 0.0 / 1.0).
std::optional<float> pvPayloadFloat(std::span<const std::uint8_t> partB);

} // namespace presonus::studiolive::gpext::protocol
