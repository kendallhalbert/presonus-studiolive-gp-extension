#pragma once

#include <optional>
#include <span>
#include <string>
#include <vector>

namespace presonus::studiolive::gpext::protocol
{

/// UCNet JM (JSON) message helpers — Subscribe / SubscriptionReply.

/// Payload after the 4-byte client identity: uint16 LE length, 0x0000, UTF-8 JSON.
std::optional<std::string> extractJmJson(std::span<const std::uint8_t> jmPayload);

bool isSubscriptionReplyJson(std::string_view json);

/// Pretty-printed Subscribe body matching the Node `craftSubscribe()` / session capture.
std::vector<std::uint8_t> createSubscribeJmPayload();

std::vector<std::uint8_t> createSubscribePacket();

std::vector<std::uint8_t> createUnsubscribePacket();

} // namespace presonus::studiolive::gpext::protocol
