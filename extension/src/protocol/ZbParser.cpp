#include "protocol/ZbParser.h"

#include "protocol/ZlibInflate.h"

namespace presonus::studiolive::gpext::protocol
{

namespace
{

std::span<const std::uint8_t> skipLeadingMarkers(std::span<const std::uint8_t> data)
{
    std::size_t offset = 0;
    if (data.size() >= 4 && data[0] == 0x65 && data[1] == 0x00)
    {
        offset += 4;
    }
    if (data.size() >= offset + 4 && data[offset] == 0x00 && data[offset + 1] == 0x00 &&
        data[offset + 2] == 0x5A && data[offset + 3] == 0x42)
    {
        offset += 4;
    }
    return data.subspan(offset);
}

std::span<const std::uint8_t> findUbjsonStart(std::span<const std::uint8_t> data)
{
    for (std::size_t i = 0; i < data.size(); ++i)
    {
        if (data[i] == 0x7B)
        {
            return data.subspan(i);
        }
    }
    return {};
}

} // namespace

std::optional<ZlibStateNode> parseZbUbjson(std::span<const std::uint8_t> ubjsonBytes)
{
    const auto jsonStart = findUbjsonStart(ubjsonBytes);
    if (jsonStart.empty())
    {
        return std::nullopt;
    }

    const auto root = parseUbjsonObject(jsonStart);
    if (!root)
    {
        return std::nullopt;
    }
    return parseZlibState(*root);
}

std::optional<ZlibStateNode> parseZbCompressed(std::span<const std::uint8_t> compressed)
{
    const auto body = skipLeadingMarkers(compressed);
    const auto inflated = zlibInflate(body);
    if (!inflated)
    {
        return std::nullopt;
    }
    return parseZbUbjson(*inflated);
}

} // namespace presonus::studiolive::gpext::protocol
