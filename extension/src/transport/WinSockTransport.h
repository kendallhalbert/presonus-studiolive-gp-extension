#pragma once

#include "transport/Transport.h"

#include <string>
#include <vector>

namespace presonus::studiolive::gpext::transport
{

/// Production TCP transport to the mixer (port 53000).
class WinSockTransport : public Transport
{
  public:
    WinSockTransport();
    ~WinSockTransport() override;

    WinSockTransport(const WinSockTransport &) = delete;
    WinSockTransport &operator=(const WinSockTransport &) = delete;

    bool connect(const std::string &host, std::uint16_t port) override;
    bool write(std::span<const std::uint8_t> bytes) override;
    void close() override;
    bool isConnected() const override;
    void pollInbound() override;

  private:
    void pumpReceive();

    static bool ensureWsaStarted();

    void *socket_{nullptr}; // SOCKET stored opaquely to keep winsock2 out of the header
    std::vector<std::uint8_t> recvScratch_;
};

} // namespace presonus::studiolive::gpext::transport
