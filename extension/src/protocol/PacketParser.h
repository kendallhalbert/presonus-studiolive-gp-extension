#pragma once

#include "protocol/MessageProtocol.h"
#include "protocol/MsParser.h"
#include "protocol/PcParser.h"
#include "protocol/PsParser.h"
#include "protocol/PvParser.h"

#include <optional>
#include <span>
#include <variant>
#include <vector>

namespace presonus::studiolive::gpext::protocol
{

using ParsedPayload =
    std::variant<PvMessage, PcMessage, PsMessage, MsMessage>;

struct ParsedWirePacket
{
    ParsedPacket wire;
    ParsedPayload payload;
};

/// Decode a full UCNet frame (header included) into wire + typed payload.
std::optional<ParsedWirePacket> parseWirePacket(std::span<const std::uint8_t> packet,
                                                bool ignoreLengthMismatch = false);

} // namespace presonus::studiolive::gpext::protocol
