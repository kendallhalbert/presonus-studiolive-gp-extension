#include "protocol/PsParser.h"

#include "protocol/ParamKey.h"

namespace presonus::studiolive::gpext::protocol
{

std::optional<PsMessage> parsePsPayload(std::span<const std::uint8_t> payload)
{
    const auto keyEnd = findNullTerminatedKeyEnd(payload);
    if (!keyEnd)
    {
        return std::nullopt;
    }

    const std::size_t valueStart = *keyEnd + 3;
    if (valueStart >= payload.size())
    {
        return std::nullopt;
    }

    std::size_t valueEnd = payload.size();
    if (valueEnd > valueStart && payload[valueEnd - 1] == 0)
    {
        --valueEnd;
    }
    if (valueEnd <= valueStart)
    {
        return std::nullopt;
    }

    PsMessage message;
    message.key = std::string(reinterpret_cast<const char *>(payload.data()), *keyEnd);
    message.value.assign(reinterpret_cast<const char *>(payload.data() + valueStart),
                       valueEnd - valueStart);
    return message;
}

} // namespace presonus::studiolive::gpext::protocol
