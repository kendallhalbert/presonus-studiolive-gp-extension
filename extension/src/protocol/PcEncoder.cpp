#include "protocol/PcEncoder.h"

#include "protocol/MessageProtocol.h"

#include <cctype>

namespace presonus::studiolive::gpext::protocol
{

namespace
{

int hexNibble(char c)
{
    if (c >= '0' && c <= '9')
    {
        return c - '0';
    }
    if (c >= 'a' && c <= 'f')
    {
        return 10 + (c - 'a');
    }
    if (c >= 'A' && c <= 'F')
    {
        return 10 + (c - 'A');
    }
    return -1;
}

} // namespace

bool parseRgbHex(std::string_view hex, std::uint8_t &r, std::uint8_t &g, std::uint8_t &b)
{
    if (hex.size() >= 1 && hex.front() == '#')
    {
        hex.remove_prefix(1);
    }
    if (hex.size() != 6)
    {
        return false;
    }

    const int rHi = hexNibble(hex[0]);
    const int rLo = hexNibble(hex[1]);
    const int gHi = hexNibble(hex[2]);
    const int gLo = hexNibble(hex[3]);
    const int bHi = hexNibble(hex[4]);
    const int bLo = hexNibble(hex[5]);
    if (rHi < 0 || rLo < 0 || gHi < 0 || gLo < 0 || bHi < 0 || bLo < 0)
    {
        return false;
    }

    r = static_cast<std::uint8_t>((rHi << 4) | rLo);
    g = static_cast<std::uint8_t>((gHi << 4) | gLo);
    b = static_cast<std::uint8_t>((bHi << 4) | bLo);
    return true;
}

std::vector<std::uint8_t> createPcPayload(const std::string &key, std::uint8_t r, std::uint8_t g,
                                          std::uint8_t b, std::uint8_t a)
{
    std::vector<std::uint8_t> payload;
    payload.insert(payload.end(), key.begin(), key.end());
    payload.push_back(0);
    payload.push_back(0);
    payload.push_back(0);
    payload.push_back(r);
    payload.push_back(g);
    payload.push_back(b);
    payload.push_back(a);
    return payload;
}

std::vector<std::uint8_t> createPcPacket(const std::string &key, std::uint8_t r, std::uint8_t g,
                                         std::uint8_t b, std::uint8_t a)
{
    return createPacket("PC", createPcPayload(key, r, g, b, a));
}

} // namespace presonus::studiolive::gpext::protocol
