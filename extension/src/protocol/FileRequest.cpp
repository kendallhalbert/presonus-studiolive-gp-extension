#include "protocol/FileRequest.h"

#include "protocol/MessageProtocol.h"

namespace presonus::studiolive::gpext::protocol
{

std::vector<std::uint8_t> createFileListRequestPayload(std::uint16_t requestId,
                                                      const std::string &path)
{
    std::vector<std::uint8_t> payload;
    payload.push_back(static_cast<std::uint8_t>(requestId & 0xFF));
    payload.push_back(static_cast<std::uint8_t>((requestId >> 8) & 0xFF));
    const std::string listPath = std::string("List") + path;
    payload.insert(payload.end(), listPath.begin(), listPath.end());
    payload.push_back(0);
    payload.push_back(0);
    return payload;
}

std::vector<std::uint8_t> createFileListRequestPacket(std::uint16_t requestId,
                                                      const std::string &path)
{
    return createPacket("FR", createFileListRequestPayload(requestId, path));
}

} // namespace presonus::studiolive::gpext::protocol
