#include "protocol/HexUtil.h"

namespace presonus::studiolive::gpext::protocol
{

std::string bytesToHex(std::span<const std::uint8_t> bytes)
{
    static constexpr char kHex[] = "0123456789abcdef";
    std::string out;
    out.reserve(bytes.size() * 2);
    for (const std::uint8_t byte : bytes)
    {
        out.push_back(kHex[byte >> 4]);
        out.push_back(kHex[byte & 0x0F]);
    }
    return out;
}

} // namespace presonus::studiolive::gpext::protocol
