#include "protocol/PvEncoder.h"

#include "protocol/MessageProtocol.h"

#include <cstring>

namespace presonus::studiolive::gpext::protocol
{

namespace
{

void appendFloatLe(std::vector<std::uint8_t> &out, float value)
{
    const auto *bytes = reinterpret_cast<const std::uint8_t *>(&value);
    out.insert(out.end(), bytes, bytes + sizeof(float));
}

} // namespace

std::vector<std::uint8_t> createPvPayload(const std::string &key, float value)
{
    std::vector<std::uint8_t> payload;
    payload.insert(payload.end(), key.begin(), key.end());
    payload.push_back(0);
    payload.push_back(0);
    payload.push_back(0);
    appendFloatLe(payload, value);
    return payload;
}

std::vector<std::uint8_t> createPvBoolPayload(const std::string &key, bool value)
{
    return createPvPayload(key, value ? 1.0F : 0.0F);
}

std::vector<std::uint8_t> createPvPacket(const std::string &key, float value)
{
    return createPacket("PV", createPvPayload(key, value));
}

std::vector<std::uint8_t> createPvBoolPacket(const std::string &key, bool value)
{
    return createPvPacket(key, value ? 1.0F : 0.0F);
}

} // namespace presonus::studiolive::gpext::protocol
