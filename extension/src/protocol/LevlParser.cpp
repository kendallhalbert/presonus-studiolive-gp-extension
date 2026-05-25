#include "protocol/LevlParser.h"

#include "protocol/MessageProtocol.h"

#include <array>
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

constexpr std::size_t kLevlPrefixSize = 20;
constexpr std::size_t kLevelsBytesPerGroup =
    static_cast<std::size_t>(kLevlChannelsPerGroup) * 2;
constexpr std::array<std::size_t, kLevlGroupCount> kGroupOffsets = {0, 72, 104, 136, 168};

} // namespace

std::optional<LevlMessage> parseLevlPacket(std::span<const std::uint8_t> packet)
{
    if (packet.size() < kLevlPrefixSize + 2)
    {
        return std::nullopt;
    }

    if (std::memcmp(packet.data(), kPacketHeader, kPacketHeaderSize) != 0)
    {
        return std::nullopt;
    }

    if (!((packet[6] == 'M' && packet[7] == 'S') || (packet[6] == 'F' && packet[7] == 'D')))
    {
        return std::nullopt;
    }

    if (std::memcmp(packet.data() + 12, "levl", 4) != 0)
    {
        return std::nullopt;
    }

    const auto data = packet.subspan(kLevlPrefixSize);
    LevlMessage message;

    for (int groupId = 0; groupId < kLevlGroupCount; ++groupId)
    {
        const std::size_t offset = kGroupOffsets[static_cast<std::size_t>(groupId)];
        if (offset >= data.size())
        {
            continue;
        }

        const std::size_t availableBytes = data.size() - offset;
        const int channelCount = static_cast<int>(
            std::min(kLevelsBytesPerGroup, availableBytes) / 2);
        if (channelCount <= 0)
        {
            continue;
        }

        std::vector<std::uint16_t> levels;
        levels.reserve(static_cast<std::size_t>(channelCount));
        for (int channel = 0; channel < channelCount; ++channel)
        {
            const std::size_t byteOffset =
                offset + static_cast<std::size_t>(channel) * 2;
            levels.push_back(readUInt16Le(data, byteOffset));
        }
        message.levelsByGroup[groupId] = std::move(levels);
    }

    if (message.levelsByGroup.empty())
    {
        return std::nullopt;
    }

    return message;
}

} // namespace presonus::studiolive::gpext::protocol
