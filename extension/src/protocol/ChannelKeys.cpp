#include "protocol/ChannelKeys.h"

#include <algorithm>
#include <cctype>

namespace presonus::studiolive::gpext::protocol
{

namespace
{

std::string toUpper(std::string_view text)
{
    std::string out(text);
    std::transform(out.begin(), out.end(), out.begin(),
                   [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    return out;
}

std::optional<std::string> wireTypeFromGpscript(std::string_view type)
{
    const std::string upper = toUpper(type);
    if (upper == "LINE")
    {
        return "line";
    }
    if (upper == "RETURN")
    {
        return "return";
    }
    if (upper == "FXRETURN")
    {
        return "fxreturn";
    }
    if (upper == "AUX")
    {
        return "aux";
    }
    if (upper == "FX")
    {
        return "fxbus";
    }
    if (upper == "SUB")
    {
        return "sub";
    }
    if (upper == "MAIN")
    {
        return "main";
    }
    if (upper == "DCA")
    {
        return "filtergroup";
    }
    if (upper == "TALKBACK")
    {
        return "talkback";
    }
    return std::nullopt;
}

std::optional<MixKind> mixKindFromGpscript(std::string_view mixType)
{
    if (mixType.empty())
    {
        return MixKind::Main;
    }
    const std::string upper = toUpper(mixType);
    if (upper == "AUX")
    {
        return MixKind::Aux;
    }
    if (upper == "FX")
    {
        return MixKind::Fx;
    }
    return std::nullopt;
}

std::string channelBaseKey(const ChannelTarget &target)
{
    return target.wireType + "/ch" + std::to_string(target.channel);
}

} // namespace

std::optional<ChannelTarget> parseChannelTarget(std::string_view type, int channel,
                                                 std::string_view mixType, int mixNumber)
{
    if (channel < 1)
    {
        return std::nullopt;
    }

    const auto wireType = wireTypeFromGpscript(type);
    if (!wireType.has_value())
    {
        return std::nullopt;
    }

    const auto mixKind = mixKindFromGpscript(mixType);
    if (!mixKind.has_value())
    {
        return std::nullopt;
    }

    if (*mixKind == MixKind::Main)
    {
        if (mixNumber != 0)
        {
            return std::nullopt;
        }
        return ChannelTarget{.wireType = *wireType,
                             .channel = channel,
                             .mixKind = MixKind::Main,
                             .mixNumber = 0};
    }

    if (mixNumber < 1)
    {
        return std::nullopt;
    }

    return ChannelTarget{.wireType = *wireType,
                         .channel = channel,
                         .mixKind = *mixKind,
                         .mixNumber = mixNumber};
}

std::string levelPvKey(const ChannelTarget &target)
{
    std::string key = channelBaseKey(target);
    switch (target.mixKind)
    {
    case MixKind::Main:
        return key + "/volume";
    case MixKind::Aux:
        return key + "/AUX" + std::to_string(target.mixNumber);
    case MixKind::Fx:
        return key + "/FX" + static_cast<char>(0x40 + target.mixNumber);
    }
    return key;
}

std::string mutePvKey(const ChannelTarget &target)
{
    std::string key = channelBaseKey(target);
    if (target.mixKind == MixKind::Main)
    {
        return key + "/mute";
    }

    const std::string mixToken =
        target.mixKind == MixKind::Aux ? "aux" : "fx";
    return key + "/assign_" + mixToken + std::to_string(target.mixNumber);
}

bool sendMuteUsesInvertedAssign(const ChannelTarget &target)
{
    return target.mixKind == MixKind::Aux || target.mixKind == MixKind::Fx;
}

} // namespace presonus::studiolive::gpext::protocol
