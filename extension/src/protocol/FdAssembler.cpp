#include "protocol/FdAssembler.h"

#include <algorithm>
#include <cstring>

namespace presonus::studiolive::gpext::protocol
{

namespace
{

std::uint16_t readUInt16Le(std::span<const std::uint8_t> bytes, std::size_t offset)
{
    return static_cast<std::uint16_t>(bytes[offset]) |
           (static_cast<std::uint16_t>(bytes[offset + 1]) << 8);
}

} // namespace

void FdAssembler::reset()
{
    buffer_.clear();
    expectedTotal_.reset();
    requestId_.reset();
    highestEnd_ = 0;
}

std::optional<FdAssemblyResult> FdAssembler::addChunk(std::span<const std::uint8_t> payload)
{
    // 14-byte FD frames with no body are stream trailers.
    if (payload.size() == kHeaderSize)
    {
        return std::nullopt;
    }

    if (payload.size() < kHeaderSize)
    {
        return std::nullopt;
    }

    const std::uint16_t chunkRequestId = readUInt16Le(payload, 0);
    const std::uint16_t chunkOffset = readUInt16Le(payload, 2);
    const std::uint16_t totalSize = readUInt16Le(payload, 6);
    const auto chunkData = payload.subspan(kHeaderSize);

    if (!requestId_)
    {
        requestId_ = chunkRequestId;
        expectedTotal_ = totalSize;
        buffer_.assign(totalSize, 0);
    }
    else if (*requestId_ != chunkRequestId || *expectedTotal_ != totalSize)
    {
        return std::nullopt;
    }

    if (chunkOffset >= buffer_.size())
    {
        return std::nullopt;
    }

    const std::size_t writeSize =
        std::min(chunkData.size(), buffer_.size() - static_cast<std::size_t>(chunkOffset));
    std::memcpy(buffer_.data() + chunkOffset, chunkData.data(), writeSize);
    highestEnd_ = std::max(highestEnd_, static_cast<std::size_t>(chunkOffset) + writeSize);

    if (highestEnd_ >= totalSize)
    {
        buffer_.resize(totalSize);
        FdAssemblyResult result;
        result.requestId = requestId_.value_or(chunkRequestId);
        result.json = std::move(buffer_);
        reset();
        return result;
    }

    return std::nullopt;
}

} // namespace presonus::studiolive::gpext::protocol
