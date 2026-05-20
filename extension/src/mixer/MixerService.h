#pragma once

#include "bridge/Logger.h"
#include "state/KvCache.h"

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <vector>

namespace presonus::studiolive::gpext::protocol
{
class MixerConnection;
}

namespace presonus::studiolive::gpext::mixer
{

/// IO-thread mixer session: TCP connect, keepalive poll, outgoing commands.
class MixerService
{
  public:
    explicit MixerService(bridge::Logger &logger);
    ~MixerService();

    MixerService(const MixerService &) = delete;
    MixerService &operator=(const MixerService &) = delete;

    bool connect(const std::string &host, std::uint16_t port = 53000);
    void disconnect();
    bool isConnected() const;

    bool setLineMute(int channel, bool muted);
    std::optional<bool> getLineMute(int channel) const;
    bool requestFileList(const std::string &path);

    /// Next `FR` / FD list request id (wraps at 16 bits).
    std::uint16_t allocateRequestId();

  private:
    using IoTask = std::function<void()>;

    void ensureThread();
    void stopThread();
    void ioLoop();
    void enqueue(IoTask task);

    bridge::Logger &logger_;
    state::KvCache stateCache_;
    std::unique_ptr<protocol::MixerConnection> connection_;
    std::thread ioThread_;
    std::mutex mutex_;
    std::vector<IoTask> tasks_;
    std::atomic<bool> running_{false};
    std::atomic<bool> connected_{false};
    std::atomic<std::uint16_t> nextRequestId_{0x1000};
};

} // namespace presonus::studiolive::gpext::mixer
