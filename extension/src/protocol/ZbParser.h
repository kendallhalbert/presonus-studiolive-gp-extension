#pragma once

#include "protocol/ZlibState.h"

#include <cstdint>
#include <optional>
#include <span>
#include <vector>

namespace presonus::studiolive::gpext::protocol
{

/// Parse a single ZB payload or reassembled CK zlib blob.
std::optional<ZlibStateNode> parseZbCompressed(std::span<const std::uint8_t> compressed);

/// Parse inflated UBJSON bytes (skips the 4-byte leading prefix on wire payloads).
std::optional<ZlibStateNode> parseZbUbjson(std::span<const std::uint8_t> ubjsonBytes);

} // namespace presonus::studiolive::gpext::protocol
