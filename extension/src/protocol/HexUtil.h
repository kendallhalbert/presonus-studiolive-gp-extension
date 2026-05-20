#pragma once

#include <cstdint>
#include <span>
#include <string>

namespace presonus::studiolive::gpext::protocol
{

std::string bytesToHex(std::span<const std::uint8_t> bytes);

} // namespace presonus::studiolive::gpext::protocol
