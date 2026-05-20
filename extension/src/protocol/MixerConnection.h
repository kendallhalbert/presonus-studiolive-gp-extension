#pragma once

#include "protocol/DataClient.h"
#include "protocol/FdAssembler.h"
#include "protocol/KeepAlive.h"
#include "protocol/MessageProtocol.h"
#include "protocol/SessionPacketDecoder.h"
#include "transport/Transport.h"

#include <chrono>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace presonus::studiolive::gpext::protocol
{

inline constexpr std::uint16_t kMixerTcpPort = 53000;

struct FdListResult
{
    std::uint16_t requestId = 0;
    std::vector<std::uint8_t> json;
};

/// TCP mixer session: deframe UCNet packets, decode params / zlib / FD lists.
class MixerConnection
{
  public:
    using SessionPacketCallback = std::function<void(SessionPacket packet)>;
    using JsonMessageCallback = std::function<void(std::string_view json)>;
    using FdListCallback = std::function<void(FdListResult result)>;
    using TimeoutCallback = std::function<void()>;

    explicit MixerConnection(std::unique_ptr<transport::Transport> transport);

    bool connect(const std::string &host, std::uint16_t port = kMixerTcpPort);
    void close();
    bool isConnected() const;

    void setSessionPacketCallback(SessionPacketCallback callback);
    void setJsonMessageCallback(JsonMessageCallback callback);
    void setFdListCallback(FdListCallback callback);
    void setTimeoutCallback(TimeoutCallback callback);

    /// Keepalive KA/FR probes are off until handshake completes.
    void setKeepAliveEnabled(bool enabled);

    bool sendRaw(std::vector<std::uint8_t> packet);

    /// Drive recv pump + keepalive probes (call from the IO thread).
    void poll(std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now());

    KeepAlive &keepAlive() { return keepAlive_; }

  private:
    std::unique_ptr<transport::Transport> transport_;
    DataClient deframer_;
    SessionPacketDecoder sessionDecoder_;
    FdAssembler fdAssembler_;
    KeepAlive keepAlive_;

    SessionPacketCallback onSessionPacket_;
    JsonMessageCallback onJsonMessage_;
    FdListCallback onFdList_;
    bool keepAliveEnabled_ = false;
};

} // namespace presonus::studiolive::gpext::protocol
