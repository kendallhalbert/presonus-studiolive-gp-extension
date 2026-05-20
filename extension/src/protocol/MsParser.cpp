#include "protocol/MsParser.h"

#include <cstring>

namespace presonus::studiolive::gpext::protocol
{

namespace
{

std::uint16_t readUInt16Le(std::span<const std::uint8_t> bytes, std::size_t offset)
{
    return static_cast<std::uint16_t>(bytes[offset]) |
           (static_cast<std::uint16_t>(bytes[offset + 1]) << 8);
}

const char *channelTypeForGroup(std::uint16_t groupNumber)
{
    switch (groupNumber)
    {
    case 0:
        return "LINE";
    case 1:
        return "RETURN";
    case 2:
        return "FXRETURN";
    case 3:
        return "TALKBACK";
    case 4:
        return "AUX";
    case 5:
        return "FX";
    case 6:
        return "SUB";
    case 7:
        return "MAIN";
    case 8:
        return "MONO";
    case 11:
        return "MASTER";
    default:
        return nullptr;
    }
}

} // namespace

std::optional<MsMessage> parseMsPayload(std::span<const std::uint8_t> payload)
{
    if (payload.size() < 8)
    {
        return std::nullopt;
    }

    if (std::memcmp(payload.data(), "fdrs", 4) != 0)
    {
        return std::nullopt;
    }

    const std::uint16_t channelCount = readUInt16Le(payload, 6);
    const std::size_t levelsBytes = static_cast<std::size_t>(channelCount) * 2;
    const std::size_t levelsOffset = 8;
    if (levelsOffset + levelsBytes > payload.size())
    {
        return std::nullopt;
    }

    FaderLevels allLevels;
    allLevels.reserve(channelCount);
    for (std::uint16_t i = 0; i < channelCount; ++i)
    {
        const std::uint16_t raw = readUInt16Le(payload, levelsOffset + static_cast<std::size_t>(i) * 2);
        allLevels.push_back(static_cast<double>(raw) / 655.35);
    }

    const std::size_t groupOffset = levelsOffset + levelsBytes;
    if (groupOffset >= payload.size())
    {
        return std::nullopt;
    }

    const std::uint8_t groupCount = payload[groupOffset];
    MsMessage message;

    for (std::uint8_t groupIndex = 0; groupIndex < groupCount; ++groupIndex)
    {
        const std::size_t descriptorOffset = groupOffset + 1 + static_cast<std::size_t>(groupIndex) * 6;
        if (descriptorOffset + 6 > payload.size())
        {
            return std::nullopt;
        }

        const std::uint16_t groupNumber = readUInt16Le(payload, descriptorOffset);
        const std::uint16_t offset = readUInt16Le(payload, descriptorOffset + 2);
        const std::uint16_t count = readUInt16Le(payload, descriptorOffset + 4);

        const char *channelType = channelTypeForGroup(groupNumber);
        if (channelType == nullptr)
        {
            continue;
        }

        if (static_cast<std::size_t>(offset) + count > allLevels.size())
        {
            return std::nullopt;
        }

        message.levelsByChannelType[channelType] =
            FaderLevels(allLevels.begin() + offset, allLevels.begin() + offset + count);
    }

    return message;
}

} // namespace presonus::studiolive::gpext::protocol
