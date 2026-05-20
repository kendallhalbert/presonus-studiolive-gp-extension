#pragma once

#include <cstddef>
#include <optional>
#include <span>
#include <string>

namespace presonus::studiolive::gpext::protocol
{

/// Split `key\\0...` at the first NUL. Returns byte index after the terminator.
std::optional<std::size_t> findNullTerminatedKeyEnd(std::span<const std::uint8_t> payload);

inline std::optional<std::string> parseNullTerminatedKey(std::span<const std::uint8_t> payload)
{
    const auto keyEnd = findNullTerminatedKeyEnd(payload);
    if (!keyEnd)
    {
        return std::nullopt;
    }
    return std::string(reinterpret_cast<const char *>(payload.data()), *keyEnd);
}

} // namespace presonus::studiolive::gpext::protocol
