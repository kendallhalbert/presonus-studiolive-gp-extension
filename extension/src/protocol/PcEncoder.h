#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace presonus::studiolive::gpext::protocol
{

/// Build a PC payload: `key\0` + 2 zero bytes + 4-byte RGBA.
std::vector<std::uint8_t> createPcPayload(const std::string &key, std::uint8_t r, std::uint8_t g,
                                          std::uint8_t b, std::uint8_t a = 0xFF);

std::vector<std::uint8_t> createPcPacket(const std::string &key, std::uint8_t r, std::uint8_t g,
                                         std::uint8_t b, std::uint8_t a = 0xFF);

/// Parse `RRGGBB` or `#RRGGBB` (alpha defaults to 0xFF).
bool parseRgbHex(std::string_view hex, std::uint8_t &r, std::uint8_t &g, std::uint8_t &b);

} // namespace presonus::studiolive::gpext::protocol
