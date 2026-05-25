#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <span>
#include <vector>

namespace presonus::studiolive::gpext::protocol
{

/// Parsed UCNet UDP `levl` meter frame (MS payload with batch type `levl`).
struct LevlMessage
{
    /// groupId 0..4 → 32-channel meter banks (see LevlParser docs).
    std::map<int, std::vector<std::uint16_t>> levelsByGroup;
};

/// Parse a raw UDP metering packet. Returns nullopt if the buffer is not a `levl` frame.
std::optional<LevlMessage> parseLevlPacket(std::span<const std::uint8_t> packet);

/// Number of meter groups in a standard `levl` frame.
inline constexpr int kLevlGroupCount = 5;

/// Channels per meter group on StudioLive III.
inline constexpr int kLevlChannelsPerGroup = 32;

/// Expected payload bytes in the largest captured JS reference frame (32R sends ~547).
inline constexpr std::size_t kLevlDataSize = 1041;

} // namespace presonus::studiolive::gpext::protocol
