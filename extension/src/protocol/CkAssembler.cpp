#include "protocol/CkAssembler.h"

#include <cstring>

namespace presonus::studiolive::gpext::protocol
{

namespace
{

std::uint32_t readUInt32Le(std::span<const std::uint8_t> bytes, std::size_t offset)
{
    return static_cast<std::uint32_t>(bytes[offset]) |
           (static_cast<std::uint32_t>(bytes[offset + 1]) << 8) |
           (static_cast<std::uint32_t>(bytes[offset + 2]) << 16) |
           (static_cast<std::uint32_t>(bytes[offset + 3]) << 24);
}

std::span<const std::uint8_t> ckChunkBody(std::span<const std::uint8_t> payload)
{
    std::size_t offset = 0;
    if (payload.size() >= 4 && payload[0] == 0x65 && payload[1] == 0x00)
    {
        offset += 4;
    }
    if (payload.size() >= offset + 4 && payload[offset] == 0x00 && payload[offset + 1] == 0x00 &&
        payload[offset + 2] == 0x5A && payload[offset + 3] == 0x42)
    {
        offset += 4;
    }
    return payload.subspan(offset);
}

} // namespace

void CkAssembler::reset()
{
    chunks_.clear();
    expectedTotal_.reset();
}

std::optional<std::vector<std::uint8_t>> CkAssembler::addChunk(std::span<const std::uint8_t> payload)
{
    const auto body = ckChunkBody(payload);
    if (body.size() < 12)
    {
        return std::nullopt;
    }

    const std::uint32_t chunkOffset = readUInt32Le(body, 0);
    const std::uint32_t totalSize = readUInt32Le(body, 4);
    const std::uint32_t chunkSize = readUInt32Le(body, 8);
    if (body.size() < 12 + chunkSize)
    {
        return std::nullopt;
    }

    if (!expectedTotal_)
    {
        expectedTotal_ = totalSize;
        chunks_.assign(totalSize, 0);
    }
    else if (*expectedTotal_ != totalSize)
    {
        return std::nullopt;
    }

    const auto chunkData = body.subspan(12, chunkSize);
    if (chunkOffset + chunkSize > chunks_.size())
    {
        return std::nullopt;
    }
    std::memcpy(chunks_.data() + chunkOffset, chunkData.data(), chunkSize);

    if (chunkOffset + chunkSize == totalSize)
    {
        std::vector<std::uint8_t> complete = std::move(chunks_);
        reset();
        return complete;
    }

    return std::nullopt;
}

} // namespace presonus::studiolive::gpext::protocol
