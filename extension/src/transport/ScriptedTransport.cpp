#include "transport/ScriptedTransport.h"

namespace presonus::studiolive::gpext::transport
{

bool ScriptedTransport::connect(const std::string & /*host*/, std::uint16_t /*port*/)
{
    connected_ = true;
    return true;
}

bool ScriptedTransport::write(std::span<const std::uint8_t> bytes)
{
    if (!connected_)
    {
        return false;
    }
    written_.emplace_back(bytes.begin(), bytes.end());
    return true;
}

void ScriptedTransport::close()
{
    connected_ = false;
}

bool ScriptedTransport::isConnected() const
{
    return connected_;
}

void ScriptedTransport::enqueueInbound(std::vector<std::uint8_t> bytes)
{
    inbound_.push(std::move(bytes));
}

void ScriptedTransport::deliverInbound()
{
    while (!inbound_.empty())
    {
        auto bytes = std::move(inbound_.front());
        inbound_.pop();
        emitReceive(bytes);
    }
}

} // namespace presonus::studiolive::gpext::transport
