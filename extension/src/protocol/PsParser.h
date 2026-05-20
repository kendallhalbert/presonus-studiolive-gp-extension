#pragma once

#include <optional>
#include <span>
#include <string>

namespace presonus::studiolive::gpext::protocol
{

struct PsMessage
{
    std::string key;
    std::string value;
};

std::optional<PsMessage> parsePsPayload(std::span<const std::uint8_t> payload);

} // namespace presonus::studiolive::gpext::protocol
