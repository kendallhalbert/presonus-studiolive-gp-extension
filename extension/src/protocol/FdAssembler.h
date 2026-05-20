#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include <vector>

namespace presonus::studiolive::gpext::protocol
{

struct FdAssemblyResult
{
    std::uint16_t requestId = 0;
    std::vector<std::uint8_t> json;
};

/// Reassembles chunked FD payloads into one JSON blob (14-byte chunk header).
class FdAssembler
{
  public:
    static constexpr std::size_t kHeaderSize = 14;

    /// Feed the FD *payload* (bytes after the 12-byte UCNet prefix).
    std::optional<FdAssemblyResult> addChunk(std::span<const std::uint8_t> payload);

    void reset();

  private:
    std::vector<std::uint8_t> buffer_;
    std::optional<std::uint16_t> expectedTotal_;
    std::optional<std::uint16_t> requestId_;
    std::size_t highestEnd_{0};
};

} // namespace presonus::studiolive::gpext::protocol
