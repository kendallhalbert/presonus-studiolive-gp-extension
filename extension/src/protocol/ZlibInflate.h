#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include <vector>

namespace presonus::studiolive::gpext::protocol
{

/// Decompress a zlib-wrapped buffer (UCNet ZB/CK payload body).
std::optional<std::vector<std::uint8_t>> zlibInflate(std::span<const std::uint8_t> compressed);

} // namespace presonus::studiolive::gpext::protocol
