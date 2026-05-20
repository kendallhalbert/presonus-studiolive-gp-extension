#include "protocol/ParamKey.h"

#include <algorithm>

namespace presonus::studiolive::gpext::protocol
{

std::optional<std::size_t> findNullTerminatedKeyEnd(std::span<const std::uint8_t> payload)
{
    const auto terminator =
        std::find(payload.begin(), payload.end(), std::uint8_t{0});
    if (terminator == payload.end())
    {
        return std::nullopt;
    }
    return static_cast<std::size_t>(terminator - payload.begin());
}

} // namespace presonus::studiolive::gpext::protocol
