#include "protocol/ZlibInflate.h"

#include <zlib.h>

#include <vector>

namespace presonus::studiolive::gpext::protocol
{

std::optional<std::vector<std::uint8_t>> zlibInflate(std::span<const std::uint8_t> compressed)
{
    if (compressed.empty())
    {
        return std::nullopt;
    }

    z_stream stream{};
    if (inflateInit2(&stream, 15 + 32) != Z_OK)
    {
        return std::nullopt;
    }

    stream.next_in = const_cast<Bytef *>(compressed.data());
    stream.avail_in = static_cast<uInt>(compressed.size());

    std::vector<std::uint8_t> output;
    output.resize(compressed.size() * 4 + 64);

    int result = Z_OK;
    while (result == Z_OK)
    {
        if (stream.total_out >= output.size())
        {
            output.resize(output.size() * 2);
        }

        stream.next_out = output.data() + stream.total_out;
        stream.avail_out = static_cast<uInt>(output.size() - stream.total_out);
        result = inflate(&stream, Z_NO_FLUSH);
        if (result == Z_STREAM_END)
        {
            break;
        }
        if (result != Z_OK)
        {
            inflateEnd(&stream);
            return std::nullopt;
        }
    }

    inflateEnd(&stream);
    if (result != Z_STREAM_END)
    {
        return std::nullopt;
    }

    output.resize(stream.total_out);
    return output;
}

} // namespace presonus::studiolive::gpext::protocol
