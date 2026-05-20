#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <span>
#include <vector>

namespace presonus::studiolive::gpext::protocol
{

/// TCP stream deframer (port of JS `DataClient.ts`).
class DataClient
{
  public:
    using PacketCallback = std::function<void(std::vector<std::uint8_t> packet)>;

    explicit DataClient(PacketCallback callback);

    void feed(std::span<const std::uint8_t> bytes);

  private:
    PacketCallback callback_;
    std::size_t remaining_{0};
    std::vector<std::uint8_t> partial_;
};

} // namespace presonus::studiolive::gpext::protocol
