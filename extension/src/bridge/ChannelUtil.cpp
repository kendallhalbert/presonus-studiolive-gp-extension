#include "bridge/ChannelUtil.h"

#include <algorithm>
#include <cctype>

namespace presonus::studiolive::gpext::bridge
{

bool isLineChannelType(std::string_view type)
{
    if (type.empty())
    {
        return false;
    }
    std::string upper(type);
    std::transform(upper.begin(), upper.end(), upper.begin(),
                   [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    return upper == "LINE";
}

} // namespace presonus::studiolive::gpext::bridge
