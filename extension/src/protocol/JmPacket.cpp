#include "protocol/JmPacket.h"

#include "protocol/MessageProtocol.h"

namespace presonus::studiolive::gpext::protocol
{

namespace
{

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

std::vector<std::uint8_t> wrapJsonPayload(std::string json)
{
    std::vector<std::uint8_t> payload;
    writeUInt16Le(payload, static_cast<std::uint16_t>(json.size()));
    payload.push_back(0);
    payload.push_back(0);
    payload.insert(payload.end(), json.begin(), json.end());
    return payload;
}

std::string escapeJsonString(std::string_view value)
{
    std::string escaped;
    escaped.reserve(value.size() + 8);
    for (const char ch : value)
    {
        switch (ch)
        {
        case '\\':
            escaped += "\\\\";
            break;
        case '"':
            escaped += "\\\"";
            break;
        default:
            escaped.push_back(ch);
            break;
        }
    }
    return escaped;
}

std::string buildRestorePresetJson(const std::string &presetFile,
                                   const std::string &presetTarget)
{
    return std::string("{\n") + " \"id\": \"RestorePreset\",\n" + " \"url\": \"presets\",\n" +
           " \"presetTarget\": \"" + escapeJsonString(presetTarget) + "\",\n" +
           " \"presetTargetSlave\": 0,\n" + " \"presetFile\": \"" +
           escapeJsonString(presetFile) + "\"\n" + "}";
}

// Matches Node JSON.stringify(obj, null, " ") — one-space indent per capture.
constexpr const char *kSubscribeJson = R"({
 "id": "Subscribe",
 "clientName": "UC-Surface",
 "clientInternalName": "ucremoteapp",
 "clientType": "StudioLive API",
 "clientDescription": "User",
 "clientIdentifier": "133d066a919ea0ea",
 "clientOptions": "perm users levl redu rtan",
 "clientEncoding": 23106
})";

constexpr const char *kUnsubscribeJson = R"({
  "id": "Unsubscribe"
})";

} // namespace

std::optional<std::string> extractJmJson(std::span<const std::uint8_t> jmPayload)
{
    if (jmPayload.size() < 5)
    {
        return std::nullopt;
    }

    const std::uint16_t jsonLength = readUInt16Le(jmPayload, 0);
    if (jmPayload.size() < 4 + jsonLength)
    {
        return std::nullopt;
    }

    return std::string(reinterpret_cast<const char *>(jmPayload.data() + 4), jsonLength);
}

bool isSubscriptionReplyJson(std::string_view json)
{
    return json.find("\"id\": \"SubscriptionReply\"") != std::string_view::npos ||
           json.find("\"id\":\"SubscriptionReply\"") != std::string_view::npos;
}

std::vector<std::uint8_t> createSubscribeJmPayload()
{
    return wrapJsonPayload(kSubscribeJson);
}

std::vector<std::uint8_t> createSubscribePacket()
{
    return createPacket("JM", createSubscribeJmPayload());
}

std::vector<std::uint8_t> createUnsubscribePacket()
{
    return createPacket("JM", wrapJsonPayload(kUnsubscribeJson));
}

std::vector<std::uint8_t> createRestorePresetJmPayload(const std::string &presetFile,
                                                       const std::string &presetTarget)
{
    return wrapJsonPayload(buildRestorePresetJson(presetFile, presetTarget));
}

std::vector<std::uint8_t> createRestorePresetPacket(const std::string &presetFile,
                                                    const std::string &presetTarget)
{
    return createPacket("JM", createRestorePresetJmPayload(presetFile, presetTarget));
}

} // namespace presonus::studiolive::gpext::protocol
