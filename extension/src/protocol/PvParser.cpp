#include "protocol/PvParser.h"

#include "protocol/ParamKey.h"

#include <cstring>

namespace presonus::studiolive::gpext::protocol
{

namespace
{

std::optional<float> readFloatLe(std::span<const std::uint8_t> bytes)
{
    if (bytes.size() < 4)
    {
        return std::nullopt;
    }
    float value = 0.0F;
    std::memcpy(&value, bytes.data(), sizeof(float));
    return value;
}

} // namespace

std::optional<PvMessage> parsePvPayload(std::span<const std::uint8_t> payload)
{
    const auto keyEnd = findNullTerminatedKeyEnd(payload);
    if (!keyEnd)
    {
        return std::nullopt;
    }

    PvMessage message;
    message.key.assign(reinterpret_cast<const char *>(payload.data()), *keyEnd);

    const std::size_t tailOffset = *keyEnd + 1;
    if (tailOffset + 2 > payload.size())
    {
        return std::nullopt;
    }

    message.partA.assign(payload.begin() + static_cast<std::ptrdiff_t>(tailOffset),
                        payload.begin() + static_cast<std::ptrdiff_t>(tailOffset + 2));
    message.partB.assign(payload.begin() + static_cast<std::ptrdiff_t>(tailOffset + 2),
                       payload.end());
    return message;
}

std::optional<float> pvPayloadFloat(std::span<const std::uint8_t> partB)
{
    return readFloatLe(partB);
}

} // namespace presonus::studiolive::gpext::protocol
