#include "protocol/SessionPacketDecoder.h"

#include "protocol/ZbParser.h"

namespace presonus::studiolive::gpext::protocol
{

namespace
{

std::optional<SessionPacket> packetFromWire(const ParsedPacket &wire, SessionPayload payload)
{
    return SessionPacket{.wire = wire, .payload = std::move(payload)};
}

} // namespace

std::optional<SessionPacket> SessionPacketDecoder::feed(std::span<const std::uint8_t> packet,
                                                        bool ignoreLengthMismatch)
{
    const auto wire = analysePacket(packet, ignoreLengthMismatch);
    if (!wire)
    {
        return std::nullopt;
    }

    if (wire->messageCode == "PV")
    {
        const auto pv = parsePvPayload(wire->payload);
        if (!pv)
        {
            return std::nullopt;
        }
        return packetFromWire(*wire, *pv);
    }
    if (wire->messageCode == "PC")
    {
        const auto pc = parsePcPayload(wire->payload);
        if (!pc)
        {
            return std::nullopt;
        }
        return packetFromWire(*wire, *pc);
    }
    if (wire->messageCode == "PS")
    {
        const auto ps = parsePsPayload(wire->payload);
        if (!ps)
        {
            return std::nullopt;
        }
        return packetFromWire(*wire, *ps);
    }
    if (wire->messageCode == "MS")
    {
        const auto ms = parseMsPayload(wire->payload);
        if (!ms)
        {
            return std::nullopt;
        }
        return packetFromWire(*wire, *ms);
    }
    if (wire->messageCode == "CK")
    {
        const auto zlibBlob = ckAssembler_.addChunk(wire->payload);
        if (!zlibBlob)
        {
            return std::nullopt;
        }
        ckAssembler_.reset();
        const auto state = parseZbCompressed(*zlibBlob);
        if (!state)
        {
            return std::nullopt;
        }
        return packetFromWire(*wire, *state);
    }
    if (wire->messageCode == "ZB")
    {
        const auto state = parseZbCompressed(wire->payload);
        if (!state)
        {
            return std::nullopt;
        }
        return packetFromWire(*wire, *state);
    }

    return std::nullopt;
}

void SessionPacketDecoder::reset()
{
    ckAssembler_.reset();
}

} // namespace presonus::studiolive::gpext::protocol
