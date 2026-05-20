#pragma once

#include <chrono>
#include <cstdint>
#include <functional>
#include <optional>
#include <vector>

namespace presonus::studiolive::gpext::protocol
{

inline constexpr std::chrono::milliseconds kKeepAliveInterval{1000};
inline constexpr std::chrono::milliseconds kKeepAliveTimeout{3000};
inline constexpr char kKeepAliveProbeKey[] = "Ftbr";

/// Empty `KA` heartbeat packet.
std::vector<std::uint8_t> createKeepAliveKaPacket();

/// `FR` probe used alongside KA; `probeId` is echoed in the FD response header.
std::vector<std::uint8_t> createKeepAliveFrPacket(std::uint16_t probeId);

/// Sends KA + FR on an interval and tracks FD probe replies (port of JS KeepAliveHelper).
class KeepAlive
{
  public:
    using SendFn = std::function<void(std::vector<std::uint8_t> packet)>;
    using TimeoutFn = std::function<void()>;

    KeepAlive(SendFn send, TimeoutFn onTimeout);

    /// Call roughly every 100 ms from the IO thread; emits probes when due.
    void poll(std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now());

    /// Notify when an incoming FD frame matches the outstanding FR probe id.
    void notifyFdResponse(std::uint16_t responseId);

    void reset();

  private:
    SendFn send_;
    TimeoutFn onTimeout_;
    std::uint16_t nextProbeId_{0};
    std::optional<std::uint16_t> pendingProbeId_;
    std::optional<std::chrono::steady_clock::time_point> lastResponseAt_;
    std::optional<std::chrono::steady_clock::time_point> lastTickAt_;
};

} // namespace presonus::studiolive::gpext::protocol
