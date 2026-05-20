#pragma once

#include <cstdint>
#include <functional>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace presonus::studiolive::gpext::transport
{

/// Abstraction over mixer TCP I/O (production WinSock, test doubles).
class Transport
{
  public:
    using ReceiveCallback = std::function<void(std::span<const std::uint8_t> bytes)>;

    virtual ~Transport() = default;

    virtual bool connect(const std::string &host, std::uint16_t port) = 0;
    virtual bool write(std::span<const std::uint8_t> bytes) = 0;
    virtual void close() = 0;
    virtual bool isConnected() const = 0;

    /// Optional inbound pump (WinSock). Default no-op for test doubles.
    virtual void pollInbound() {}

    void setReceiveCallback(ReceiveCallback callback) { receive_ = std::move(callback); }

  protected:
    void emitReceive(std::span<const std::uint8_t> bytes) const
    {
        if (receive_)
        {
            receive_(bytes);
        }
    }

    ReceiveCallback receive_;
};

} // namespace presonus::studiolive::gpext::transport
