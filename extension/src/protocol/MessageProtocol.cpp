#include "protocol/MessageProtocol.h"

#include <cstring>

namespace presonus::studiolive::gpext::protocol
{

namespace
{

bool headerMatches(std::span<const std::uint8_t> packet)
{
    return packet.size() >= kPacketHeaderSize &&
           std::memcmp(packet.data(), kPacketHeader, kPacketHeaderSize) == 0;
}

std::uint16_t readUInt16Le(std::span<const std::uint8_t> bytes, std::size_t offset)
{
    return static_cast<std::uint16_t>(bytes[offset]) |
           (static_cast<std::uint16_t>(bytes[offset + 1]) << 8);
}

void writeUInt16Le(std::vector<std::uint8_t> &out, std::uint16_t value)
{
    out.push_back(static_cast<std::uint8_t>(value & 0xFF));
    out.push_back(static_cast<std::uint8_t>((value >> 8) & 0xFF));
}

} // namespace

std::optional<ParsedPacket> analysePacket(std::span<const std::uint8_t> packet,
                                          bool ignoreLengthMismatch)
{
    if (!headerMatches(packet) || packet.size() < kPacketPrefixSize)
    {
        return std::nullopt;
    }

    const std::uint16_t payloadLength = readUInt16Le(packet, 4);
    const std::size_t expectedSize = static_cast<std::size_t>(payloadLength) + 6;
    if (expectedSize != packet.size() && !ignoreLengthMismatch)
    {
        return std::nullopt;
    }

    ParsedPacket result;
    result.messageCode.assign(reinterpret_cast<const char *>(packet.data() + 6), 2);
    result.customA = packet[8];
    result.customB = packet[10];
    result.payload.assign(packet.begin() + static_cast<std::ptrdiff_t>(kPacketPrefixSize),
                          packet.end());
    return result;
}

std::vector<std::uint8_t> createPacket(const std::string &messageCode,
                                       std::span<const std::uint8_t> payload,
                                       std::uint8_t customA,
                                       std::uint8_t customB)
{
    if (messageCode.size() != 2)
    {
        return {};
    }

    const std::uint16_t innerLength =
        static_cast<std::uint16_t>(2 + 4 + payload.size());
    std::vector<std::uint8_t> out;
    out.reserve(kPacketPrefixSize + payload.size());
    out.insert(out.end(), std::begin(kPacketHeader), std::end(kPacketHeader));
    writeUInt16Le(out, innerLength);
    out.push_back(static_cast<std::uint8_t>(messageCode[0]));
    out.push_back(static_cast<std::uint8_t>(messageCode[1]));
    out.push_back(customA);
    out.push_back(0);
    out.push_back(customB);
    out.push_back(0);
    out.insert(out.end(), payload.begin(), payload.end());
    return out;
}

} // namespace presonus::studiolive::gpext::protocol
