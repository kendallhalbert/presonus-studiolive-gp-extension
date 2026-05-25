#pragma once

#include "bridge/Logger.h"
#include "protocol/LevlParser.h"
#include "state/MeterCache.h"

#include <cstdint>
#include <functional>

namespace presonus::studiolive::gpext::transport
{

/// UDP listener for UCNet `levl` metering frames sent by the mixer.
class MeterListener
{
  public:
    using FrameCallback = std::function<void(const protocol::LevlMessage &)>;

    explicit MeterListener(bridge::Logger &logger);
    ~MeterListener();

    MeterListener(const MeterListener &) = delete;
    MeterListener &operator=(const MeterListener &) = delete;

    bool start(std::uint16_t port);
    void stop();
    bool isRunning() const { return running_; }

    /// Non-blocking receive + parse. Returns true when a frame was applied.
    bool poll(state::MeterCache &cache, FrameCallback onFrame = {});

  private:
    bridge::Logger &logger_;
    void *socketHandle_{nullptr};
    std::uint16_t port_{0};
    bool running_{false};
};

} // namespace presonus::studiolive::gpext::transport
