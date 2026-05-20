#include "protocol/MixerConnection.h"

#include "protocol/JmPacket.h"

namespace presonus::studiolive::gpext::protocol
{

namespace
{

std::uint16_t readUInt16Le(std::span<const std::uint8_t> bytes)
{
    return static_cast<std::uint16_t>(bytes[0]) |
           (static_cast<std::uint16_t>(bytes[1]) << 8);
}

} // namespace

MixerConnection::MixerConnection(std::unique_ptr<transport::Transport> transport)
    : transport_(std::move(transport)),
      deframer_([this](std::vector<std::uint8_t> packet) {
          const auto wire = analysePacket(packet);
          if (!wire)
          {
              return;
          }

          if (wire->messageCode == "JM")
          {
              if (const auto json = extractJmJson(wire->payload))
              {
                  if (onJsonMessage_)
                  {
                      onJsonMessage_(*json);
                  }
              }
              return;
          }

          if (wire->messageCode == "FD")
          {
              if (!wire->payload.empty())
              {
                  keepAlive_.notifyFdResponse(readUInt16Le(wire->payload));
              }

              if (const auto jsonBytes = fdAssembler_.addChunk(wire->payload))
              {
                  if (onFdList_)
                  {
                      onFdList_(FdListResult{*jsonBytes});
                  }
              }
              return;
          }

          if (const auto session = sessionDecoder_.feed(packet))
          {
              if (onSessionPacket_)
              {
                  onSessionPacket_(*session);
              }
          }
      }),
      keepAlive_(
          [this](std::vector<std::uint8_t> packet) { sendRaw(std::move(packet)); },
          [this]() { close(); })
{
    transport_->setReceiveCallback([this](std::span<const std::uint8_t> bytes) {
        deframer_.feed(bytes);
    });
}

bool MixerConnection::connect(const std::string &host, std::uint16_t port)
{
    keepAlive_.reset();
    keepAliveEnabled_ = false;
    sessionDecoder_.reset();
    fdAssembler_.reset();
    return transport_->connect(host, port);
}

void MixerConnection::close()
{
    keepAlive_.reset();
    transport_->close();
}

bool MixerConnection::isConnected() const
{
    return transport_->isConnected();
}

void MixerConnection::setSessionPacketCallback(SessionPacketCallback callback)
{
    onSessionPacket_ = std::move(callback);
}

void MixerConnection::setJsonMessageCallback(JsonMessageCallback callback)
{
    onJsonMessage_ = std::move(callback);
}

void MixerConnection::setKeepAliveEnabled(bool enabled)
{
    keepAliveEnabled_ = enabled;
    if (!enabled)
    {
        keepAlive_.reset();
    }
}

void MixerConnection::setFdListCallback(FdListCallback callback)
{
    onFdList_ = std::move(callback);
}

void MixerConnection::setTimeoutCallback(TimeoutCallback callback)
{
    keepAlive_ = KeepAlive(
        [this](std::vector<std::uint8_t> packet) { sendRaw(std::move(packet)); },
        [this, callback = std::move(callback)]() {
            if (callback)
            {
                callback();
            }
            close();
        });
}

bool MixerConnection::sendRaw(std::vector<std::uint8_t> packet)
{
    const bool ok = transport_->write(packet);
    transport_->pollInbound();
    return ok;
}

void MixerConnection::poll(const std::chrono::steady_clock::time_point now)
{
    transport_->pollInbound();
    if (keepAliveEnabled_)
    {
        keepAlive_.poll(now);
    }
}

} // namespace presonus::studiolive::gpext::protocol
