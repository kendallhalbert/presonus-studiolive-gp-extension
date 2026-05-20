#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace presonus::studiolive::gpext::protocol
{

inline constexpr std::uint8_t kPacketHeader[] = {0x55, 0x43, 0x00, 0x01};
inline constexpr std::size_t kPacketHeaderSize = 4;
inline constexpr std::size_t kPacketPrefixSize = 12; // header + length + code + c-bytes
inline constexpr std::uint8_t kCByteA = 0x68;
inline constexpr std::uint8_t kCByteB = 0x65;

struct ParsedPacket
{
    std::string messageCode;
    std::uint8_t customA{kCByteA};
    std::uint8_t customB{kCByteB};
    std::vector<std::uint8_t> payload;
};

/// Decode a wire buffer into message code + payload (bytes after the C-byte pair).
std::optional<ParsedPacket> analysePacket(std::span<const std::uint8_t> packet,
                                          bool ignoreLengthMismatch = false);

/// Craft a UCNet packet (header + length + code + identity + payload).
std::vector<std::uint8_t> createPacket(const std::string &messageCode,
                                       std::span<const std::uint8_t> payload,
                                       std::uint8_t customA = kCByteA,
                                       std::uint8_t customB = kCByteB);

} // namespace presonus::studiolive::gpext::protocol
