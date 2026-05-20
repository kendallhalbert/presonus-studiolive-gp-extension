#include "protocol/ConnectionHandshake.h"

#include "protocol/JmPacket.h"
#include "protocol/ZlibState.h"

#include <variant>

namespace presonus::studiolive::gpext::protocol
{

void ConnectionHandshake::onSessionPacket(const SessionPacket &packet)
{
    if (std::holds_alternative<ZlibStateNode>(packet.payload))
    {
        progress_.zlibStateReceived = true;
    }
}

void ConnectionHandshake::onJmJson(std::string_view json)
{
    if (isSubscriptionReplyJson(json))
    {
        progress_.subscriptionReplyReceived = true;
    }
}

} // namespace presonus::studiolive::gpext::protocol
