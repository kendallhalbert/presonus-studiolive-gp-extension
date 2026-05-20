#pragma once

#include "protocol/SessionPacketDecoder.h"

#include <string_view>

namespace presonus::studiolive::gpext::protocol
{

struct HandshakeProgress
{
    bool zlibStateReceived = false;
    bool subscriptionReplyReceived = false;

    bool complete() const { return zlibStateReceived && subscriptionReplyReceived; }
};

/// Tracks ZB/CK state + JM SubscriptionReply during UCNet connect.
class ConnectionHandshake
{
  public:
    void onSessionPacket(const SessionPacket &packet);
    void onJmJson(std::string_view json);

    const HandshakeProgress &progress() const { return progress_; }
    void reset() { progress_ = {}; }

  private:
    HandshakeProgress progress_;
};

} // namespace presonus::studiolive::gpext::protocol
