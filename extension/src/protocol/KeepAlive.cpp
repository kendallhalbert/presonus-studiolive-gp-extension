#include "protocol/KeepAlive.h"

#include "protocol/MessageProtocol.h"

#include <cstring>

namespace presonus::studiolive::gpext::protocol
{

namespace
{

void appendUInt16Le(std::vector<std::uint8_t> &out, std::uint16_t value)
{
    out.push_back(static_cast<std::uint8_t>(value & 0xFF));
    out.push_back(static_cast<std::uint8_t>((value >> 8) & 0xFF));
}

} // namespace

std::vector<std::uint8_t> createKeepAliveKaPacket()
{
    return createPacket("KA", {});
}

std::vector<std::uint8_t> createKeepAliveFrPacket(std::uint16_t probeId)
{
    std::vector<std::uint8_t> payload;
    appendUInt16Le(payload, probeId);
    payload.insert(payload.end(), reinterpret_cast<const std::uint8_t *>(kKeepAliveProbeKey),
                   reinterpret_cast<const std::uint8_t *>(kKeepAliveProbeKey) + 4);
    payload.push_back(0);
    payload.push_back(0);
    return createPacket("FR", payload);
}

KeepAlive::KeepAlive(SendFn send, TimeoutFn onTimeout)
    : send_(std::move(send)), onTimeout_(std::move(onTimeout))
{
}

void KeepAlive::reset()
{
    nextProbeId_ = 0;
    pendingProbeId_.reset();
    lastResponseAt_.reset();
    lastTickAt_.reset();
}

void KeepAlive::notifyFdResponse(std::uint16_t responseId)
{
    if (!pendingProbeId_ || *pendingProbeId_ != responseId)
    {
        return;
    }
    pendingProbeId_.reset();
    lastResponseAt_ = std::chrono::steady_clock::now();
}

void KeepAlive::poll(std::chrono::steady_clock::time_point now)
{
    if (lastResponseAt_ && now - *lastResponseAt_ > kKeepAliveTimeout)
    {
        if (onTimeout_)
        {
            onTimeout_();
        }
        reset();
        return;
    }

    if (lastTickAt_ && now - *lastTickAt_ < kKeepAliveInterval)
    {
        return;
    }

    lastTickAt_ = now;
    if (!send_)
    {
        return;
    }

    send_(createKeepAliveKaPacket());

    const std::uint16_t probeId = nextProbeId_++;
    pendingProbeId_ = probeId;
    if (!lastResponseAt_)
    {
        lastResponseAt_ = now;
    }
    send_(createKeepAliveFrPacket(probeId));
}

} // namespace presonus::studiolive::gpext::protocol
