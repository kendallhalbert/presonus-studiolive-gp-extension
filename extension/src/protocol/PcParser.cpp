#include "protocol/PcParser.h"

#include "protocol/HexUtil.h"
#include "protocol/ParamKey.h"

namespace presonus::studiolive::gpext::protocol
{

std::optional<PcMessage> parsePcPayload(std::span<const std::uint8_t> payload)
{
    const auto keyEnd = findNullTerminatedKeyEnd(payload);
    if (!keyEnd)
    {
        return std::nullopt;
    }

    const std::size_t valueOffset = *keyEnd + 3;
    if (valueOffset + 4 > payload.size())
    {
        return std::nullopt;
    }

    PcMessage message;
    message.key = std::string(reinterpret_cast<const char *>(payload.data()), *keyEnd);
    message.valueHex =
        bytesToHex(payload.subspan(valueOffset, 4));
    return message;
}

} // namespace presonus::studiolive::gpext::protocol
