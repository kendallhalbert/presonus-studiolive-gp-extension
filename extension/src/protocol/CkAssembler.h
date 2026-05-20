#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include <vector>

namespace presonus::studiolive::gpext::protocol
{

/// Reassembles chunked CK payloads into one zlib blob (port of JS `handleCKPacket`).
class CkAssembler
{
  public:
    /// Feed the CK *payload* (bytes after the 12-byte UCNet prefix).
    /// Returns the concatenated zlib bytes when the final chunk arrives.
    std::optional<std::vector<std::uint8_t>> addChunk(std::span<const std::uint8_t> payload);

    void reset();

  private:
    std::vector<std::uint8_t> chunks_;
    std::optional<std::uint32_t> expectedTotal_;
};

} // namespace presonus::studiolive::gpext::protocol
