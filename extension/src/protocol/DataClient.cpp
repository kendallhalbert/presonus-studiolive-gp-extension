#include "protocol/DataClient.h"

#include "protocol/MessageProtocol.h"

#include <algorithm>
#include <cstring>

namespace presonus::studiolive::gpext::protocol
{

namespace
{

bool headerMatches(std::span<const std::uint8_t> bytes)
{
    return bytes.size() >= kPacketHeaderSize &&
           std::memcmp(bytes.data(), kPacketHeader, kPacketHeaderSize) == 0;
}

std::uint16_t payloadLengthFromHeader(std::span<const std::uint8_t> bytes)
{
    return static_cast<std::uint16_t>(bytes[4]) |
           (static_cast<std::uint16_t>(bytes[5]) << 8);
}

} // namespace

DataClient::DataClient(PacketCallback callback) : callback_(std::move(callback))
{
}

void DataClient::feed(std::span<const std::uint8_t> bytes)
{
    std::vector<std::uint8_t> frame(bytes.begin(), bytes.end());

    while (!frame.empty())
    {
        if (remaining_ == 0 && headerMatches(frame))
        {
            const std::uint16_t innerLength = payloadLengthFromHeader(frame);
            const std::size_t packetSize = static_cast<std::size_t>(innerLength) + 6;

            if (frame.size() < packetSize)
            {
                partial_.assign(frame.begin(), frame.end());
                remaining_ = packetSize - frame.size();
                return;
            }

            if (callback_)
            {
                callback_(std::vector<std::uint8_t>(frame.begin(),
                                                    frame.begin() + static_cast<std::ptrdiff_t>(packetSize)));
            }
            frame.erase(frame.begin(), frame.begin() + static_cast<std::ptrdiff_t>(packetSize));
            continue;
        }

        if (remaining_ > 0)
        {
            const std::size_t extractCount = std::min(remaining_, frame.size());
            partial_.insert(partial_.end(), frame.begin(),
                            frame.begin() + static_cast<std::ptrdiff_t>(extractCount));
            remaining_ -= extractCount;
            frame.erase(frame.begin(), frame.begin() + static_cast<std::ptrdiff_t>(extractCount));

            if (remaining_ == 0)
            {
                if (callback_)
                {
                    callback_(partial_);
                }
                partial_.clear();
            }
            continue;
        }

        return;
    }
}

} // namespace presonus::studiolive::gpext::protocol
