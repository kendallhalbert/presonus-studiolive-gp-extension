#include "protocol/PacketParser.h"

namespace presonus::studiolive::gpext::protocol
{

std::optional<ParsedWirePacket> parseWirePacket(std::span<const std::uint8_t> packet,
                                                bool ignoreLengthMismatch)
{
    const auto wire = analysePacket(packet, ignoreLengthMismatch);
    if (!wire)
    {
        return std::nullopt;
    }

    ParsedWirePacket result{.wire = *wire};

    if (wire->messageCode == "PV")
    {
        const auto pv = parsePvPayload(wire->payload);
        if (!pv)
        {
            return std::nullopt;
        }
        result.payload = *pv;
    }
    else if (wire->messageCode == "PC")
    {
        const auto pc = parsePcPayload(wire->payload);
        if (!pc)
        {
            return std::nullopt;
        }
        result.payload = *pc;
    }
    else if (wire->messageCode == "PS")
    {
        const auto ps = parsePsPayload(wire->payload);
        if (!ps)
        {
            return std::nullopt;
        }
        result.payload = *ps;
    }
    else if (wire->messageCode == "MS")
    {
        const auto ms = parseMsPayload(wire->payload);
        if (!ms)
        {
            return std::nullopt;
        }
        result.payload = *ms;
    }
    else
    {
        return std::nullopt;
    }

    return result;
}

} // namespace presonus::studiolive::gpext::protocol
