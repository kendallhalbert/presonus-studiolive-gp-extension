#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace presonus::studiolive::gpext::protocol
{

/// Build a PV payload: `key\0` + 2-byte partA (zeros) + 4-byte float partB.
std::vector<std::uint8_t> createPvPayload(const std::string &key, float value);

std::vector<std::uint8_t> createPvBoolPayload(const std::string &key, bool value);

/// Outgoing client PV packet (custom bytes 0x68 / 0x65).
std::vector<std::uint8_t> createPvPacket(const std::string &key, float value);

std::vector<std::uint8_t> createPvBoolPacket(const std::string &key, bool value);

/// `line/ch{n}/mute` for 1-based channel index.
std::string lineChannelMuteKey(int channel);

} // namespace presonus::studiolive::gpext::protocol
