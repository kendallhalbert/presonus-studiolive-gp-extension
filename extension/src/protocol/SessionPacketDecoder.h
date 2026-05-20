#pragma once

#include "protocol/CkAssembler.h"
#include "protocol/MessageProtocol.h"
#include "protocol/MsParser.h"
#include "protocol/PcParser.h"
#include "protocol/PsParser.h"
#include "protocol/PvParser.h"
#include "protocol/ZlibState.h"

#include <optional>
#include <span>
#include <variant>
#include <vector>

namespace presonus::studiolive::gpext::protocol
{

using SessionPayload =
    std::variant<PvMessage, PcMessage, PsMessage, MsMessage, ZlibStateNode>;

struct SessionPacket
{
    ParsedPacket wire;
    SessionPayload payload;
};

/// Stateful UCNet decoder: reassembles CK chunks, inflates ZB/CK zlib payloads.
class SessionPacketDecoder
{
  public:
    /// Returns a decoded packet, or `nullopt` when more CK chunks are needed.
    std::optional<SessionPacket> feed(std::span<const std::uint8_t> packet,
                                      bool ignoreLengthMismatch = false);

    void reset();

  private:
    CkAssembler ckAssembler_;
};

} // namespace presonus::studiolive::gpext::protocol
