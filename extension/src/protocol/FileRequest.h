#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace presonus::studiolive::gpext::protocol
{

/// `FR` list request (port of JS `sendList`): id + `List` + path + trailing nulls.
std::vector<std::uint8_t> createFileListRequestPayload(std::uint16_t requestId,
                                                      const std::string &path);

std::vector<std::uint8_t> createFileListRequestPacket(std::uint16_t requestId,
                                                      const std::string &path);

} // namespace presonus::studiolive::gpext::protocol
