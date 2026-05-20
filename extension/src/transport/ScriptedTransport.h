#pragma once

#include "transport/Transport.h"

#include <queue>
#include <vector>

namespace presonus::studiolive::gpext::transport
{

/// In-memory transport for unit tests: scripted inbound bytes, records outbound writes.
class ScriptedTransport : public Transport
{
  public:
    bool connect(const std::string &host, std::uint16_t port) override;
    bool write(std::span<const std::uint8_t> bytes) override;
    void close() override;
    bool isConnected() const override;

    void enqueueInbound(std::vector<std::uint8_t> bytes);
    void deliverInbound();

    const std::vector<std::vector<std::uint8_t>> &written() const { return written_; }

  private:
    bool connected_{false};
    std::queue<std::vector<std::uint8_t>> inbound_;
    std::vector<std::vector<std::uint8_t>> written_;
};

} // namespace presonus::studiolive::gpext::transport
